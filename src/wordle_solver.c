#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/wordle_solver.h"

#define WORDLEN 5
#define MAX_ATTEMPTS 6

/* compute Wordle-style feedback: 'g' green, 'y' yellow, 'b' black.
   fb must be at least WORDLEN+1. */
static void compute_feedback(const char *secret, const char *guess, char *fb) {
    int i;
    int secret_count[26] = {0};
    int green[WORDLEN] = {0};

    for (i = 0; i < WORDLEN; ++i) fb[i] = '?';

    /* mark greens */
    for (i = 0; i < WORDLEN; ++i) {
        if (guess[i] == secret[i]) {
            fb[i] = 'g';
            green[i] = 1;
        }
    }

    /* count remaining letters in secret */
    for (i = 0; i < WORDLEN; ++i) {
        if (!green[i]) {
            char c = secret[i];
            if ('A' <= c && c <= 'Z') c = c - 'A' + 'a';
            if ('a' <= c && c <= 'z') secret_count[c - 'a']++;
        }
    }

    /* yellows / blacks */
    for (i = 0; i < WORDLEN; ++i) {
        if (fb[i] == 'g') continue;
        char c = guess[i];
        if ('A' <= c && c <= 'Z') c = c - 'A' + 'a';
        if (c < 'a' || c > 'z') { fb[i] = 'b'; continue; }
        if (secret_count[c - 'a'] > 0) {
            fb[i] = 'y';
            secret_count[c - 'a']--;
        } else {
            fb[i] = 'b';
        }
    }
    fb[WORDLEN] = '\0';
}

/* load words (5-letter lowercase a-z) from filename */
static char **load_words(const char *filename, int *out_count) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        *out_count = 0;
        return NULL;
    }

    char buf[256];
    char **arr = NULL;
    int cap = 0, n = 0;

    while (fgets(buf, sizeof(buf), f)) {
        /* trim leading/trailing whitespace */
        char *p = buf;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;
        char *end = p + strlen(p) - 1;
        while (end >= p && isspace((unsigned char)*end)) { *end = '\0'; end--; }

        if ((int)strlen(p) != WORDLEN) continue;

        int valid = 1;
        for (int i = 0; i < WORDLEN; ++i) {
            if (!isalpha((unsigned char)p[i])) { valid = 0; break; }
            p[i] = (char)tolower((unsigned char)p[i]);
        }
        if (!valid) continue;

        if (n >= cap) {
            cap = cap ? cap * 2 : 1024;
            char **tmp = realloc(arr, cap * sizeof(char *));
            if (!tmp) { perror("realloc"); break; }
            arr = tmp;
        }
        arr[n] = strdup(p);
        if (!arr[n]) { perror("strdup"); break; }
        n++;
    }

    fclose(f);
    *out_count = n;
    return arr;
}

/* free list */
static void free_words(char **arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; ++i) free(arr[i]);
    free(arr);
}

/* return 1 if candidate (as secret) would produce given_fb when guess is guess */
static int matches_feedback(const char *candidate, const char *guess, const char *given_fb) {
    char fb[WORDLEN + 1];
    compute_feedback(candidate, guess, fb);
    return strcmp(fb, given_fb) == 0;
}

/* filter candidates in-place. returns new count. frees removed strings. */
static int filter_candidates(char **cands, int cand_count, const char *guess, const char *fb) {
    int write = 0;
    for (int i = 0; i < cand_count; ++i) {
        if (matches_feedback(cands[i], guess, fb)) {
            if (write != i) {
                free(cands[write]);
                cands[write] = cands[i];
            }
            write++;
        } else {
            free(cands[i]);
        }
    }
    return write;
}

/* check if word is in guessed list */
static int was_guessed(char **guessed, int guessed_count, const char *w) {
    for (int i = 0; i < guessed_count; ++i) if (strcmp(guessed[i], w) == 0) return 1;
    return 0;
}

/* choose next unguessed candidate (first one) or NULL */
static char *choose_next(char **cands, int cand_count, char **guessed, int guessed_count) {
    for (int i = 0; i < cand_count; ++i) {
        if (!was_guessed(guessed, guessed_count, cands[i])) return cands[i];
    }
    return NULL;
}

/* The API function you declared in header.
   - target != NULL : automatic mode (solver computes feedback using target)
   - target == NULL : interactive mode (asks user to type feedback after each guess)
   - input : path to word-list file (e.g. "word.txt") */
void solve(const char* target, const char* input) {
    const char *initial[3] = { "plumb", "crane", "sight" };
    char **words = NULL;
    int words_count = 0;

    words = load_words(input ? input : "words.txt", &words_count);
    if (!words || words_count == 0) {
        fprintf(stderr, "No words loaded from %s\n", input ? input : "words.txt");
        free_words(words, words_count);
        return;
    }

    /* make candidate copy */
    char **cands = malloc(words_count * sizeof(char*));
    if (!cands) { perror("malloc"); free_words(words, words_count); return; }
    int cand_count = 0;
    for (int i = 0; i < words_count; ++i) cands[cand_count++] = strdup(words[i]);

    char **guessed = malloc((words_count + 3) * sizeof(char*));
    int guessed_count = 0;

    char fb[WORDLEN + 1];
    int attempt;

    printf("Solver started (%s mode). You have %d attempts.\n",
           (target ? "automatic" : "interactive"), MAX_ATTEMPTS);

    for (attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        const char *to_guess = NULL;
        if (attempt < 3) {
            to_guess = initial[attempt];
        } else {
            char *pick = choose_next(cands, cand_count, guessed, guessed_count);
            if (!pick) {
                printf("No unguessed candidate available. Stopping early.\n");
                break;
            }
            to_guess = pick;
        }

        if (was_guessed(guessed, guessed_count, to_guess)) {
            /* shouldn't usually happen for initial guesses, but skip if so */
            attempt--;
            continue;
        }

        printf("\nAttempt %d: %s\n", attempt + 1, to_guess);

        if (target) {
            /* automatic: compute feedback from target */
            compute_feedback(target, to_guess, fb);
            printf("Feedback (auto): %s\n", fb);
        } else {
            /* interactive: ask user for feedback */
            printf("Enter feedback (5 chars g/y/b): ");
            if (scanf("%5s", fb) != 1) {
                fprintf(stderr, "Input error\n");
                break;
            }
            for (int i = 0; i < WORDLEN; ++i) fb[i] = (char)tolower((unsigned char)fb[i]);
            fb[WORDLEN] = '\0';
        }

        guessed[guessed_count++] = strdup(to_guess);

        if (strcmp(fb, "ggggg") == 0) {
            printf("Solved! Word is: %s (found on attempt %d)\n", to_guess, attempt + 1);
            break;
        }

        /* filter */
        cand_count = filter_candidates(cands, cand_count, to_guess, fb);
        printf("Candidates remaining: %d\n", cand_count);
        int show = (cand_count < 10) ? cand_count : 10;
        for (int i = 0; i < show; ++i) printf(" - %s\n", cands[i]);

        if (cand_count == 0) {
            printf("No candidates remain after filtering — check your feedback.\n");
            break;
        }
    }

    if (attempt >= MAX_ATTEMPTS) {
        printf("Reached maximum attempts (%d).\n", MAX_ATTEMPTS);
    }

    for (int i = 0; i < guessed_count; ++i) free(guessed[i]);
    free(guessed);
    for (int i = 0; i < cand_count; ++i) free(cands[i]);
    free(cands);
    free_words(words, words_count);
}

/* convenience wrapper for main (interactive solver) */
void solve_wordle() {
    solve(NULL, "word.txt");
}
