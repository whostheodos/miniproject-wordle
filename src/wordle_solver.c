// Full automated Wordle solver that:
// - loads a word list (5-letter words)
// - always plays first three guesses: "plumb", "crane", "sight"
// - then uses an elimination strategy: for each candidate guess, simulate feedback
//   against all remaining possible secrets and choose the guess that minimizes
//   the expected remaining candidate-size (sum(count^2)/N heuristic).
// Two modes:
// - automatic: provide the secret; feedback is computed by the program
// - interactive: user types feedback for each guess (use 'G' for green, 'Y' for yellow, 'B' for absent/black)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/wordle_solver.h"


#define WORD_LEN 5
#define MAX_WORDS 20000
#define MAX_ATTEMPTS 6

static void strlower(char *s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

/* produce Wordle-style feedback:
   'G' = green (correct position)
   'Y' = yellow (present but wrong position)
   'B' = absent
   feedback must point to a buffer of length >= WORD_LEN+1 */
static void get_feedback(const char *secret, const char *guess, char *feedback) {
    int used_secret[WORD_LEN] = {0};
    for (int i = 0; i < WORD_LEN; ++i) feedback[i] = 'B';
    feedback[WORD_LEN] = '\0';

    // Greens
    for (int i = 0; i < WORD_LEN; ++i) {
        if (guess[i] == secret[i]) {
            feedback[i] = 'G';
            used_secret[i] = 1;
        }
    }
    // Yellows
    for (int i = 0; i < WORD_LEN; ++i) {
        if (feedback[i] == 'G') continue;
        for (int j = 0; j < WORD_LEN; ++j) {
            if (!used_secret[j] && guess[i] == secret[j]) {
                feedback[i] = 'Y';
                used_secret[j] = 1;
                break;
            }
        }
    }
}

/* Load word list from file; returns malloc'd array of strings and sets out_count.
   Accepts multiple candidate paths in caller; only 5-letter words kept. */
static char **load_wordlist_path(const char *path, int *out_count) {
    FILE *f = fopen(path, "r");
    if (!f) { 
        *out_count = 0; 
        return NULL; 
    }
    
    char **list = malloc(sizeof(char*) * MAX_WORDS);
    if (!list) { fclose(f); *out_count = 0; return NULL; }

    char buf[256];
    int count = 0;
    while (fgets(buf, sizeof(buf), f)) {
        // trim newline and whitespace
        char *p = buf;
        while (*p && isspace((unsigned char)*p)) ++p;
        char *end = p + strlen(p) - 1;
        while (end >= p && isspace((unsigned char)*end)) *end-- = '\0';
        if ((int)strlen(p) != WORD_LEN) continue;
        strlower(p);
        list[count] = malloc(WORD_LEN + 1);
        strcpy(list[count], p);
        ++count;
        if (count >= MAX_WORDS) break;
    }
    fclose(f);
    *out_count = count;
    return list;
}

static void free_wordlist(char **list, int count) {
    if (!list) return;
    for (int i = 0; i < count; ++i) free(list[i]);
    free(list);
}

/* Check if candidate would produce exactly feedback when compared to guess */
static int candidate_matches_feedback(const char *candidate, const char *guess, const char *feedback) {
    char fb[WORD_LEN+1];
    get_feedback(candidate, guess, fb);
    return strcmp(fb, feedback) == 0;
}

/* Filter in-place; returns new count. Freed entries are free()d. */
static int filter_candidates_inplace(char **cands, int count, const char *guess, const char *feedback) {
    int w = 0;
    for (int i = 0; i < count; ++i) {
        if (candidate_matches_feedback(cands[i], guess, feedback)) {
            cands[w++] = cands[i];
        } else {
            free(cands[i]);
        }
    }
    return w;
}

static int pick_best_guess_index(char **cands, int count) {
    if (count <= 1) return 0;
    double best_score = 1e308;
    int best_idx = 0;

    // For patterns we store strings in an array and counts in parallel.
    // This is O(N^2) but acceptable for typical word lists (~2-3k).
    char pattern[WORD_LEN+1];
    for (int gi = 0; gi < count; ++gi) {
        double score_sum = 0.0;
        // reset pattern arrays for each guess
        char **patterns = malloc(sizeof(char*) * count);
        int *pcounts = malloc(sizeof(int) * count);
        int pnum = 0;

        for (int si = 0; si < count; ++si) {
            get_feedback(cands[si], cands[gi], pattern); // pattern when guess=cands[gi] vs secret=cands[si]
            // find pattern
            int found = 0;
            for (int p = 0; p < pnum; ++p) {
                if (strcmp(patterns[p], pattern) == 0) { pcounts[p]++; found = 1; break; }
            }
            if (!found) {
                patterns[pnum] = malloc(WORD_LEN+1);
                strcpy(patterns[pnum], pattern);
                pcounts[pnum] = 1;
                pnum++;
            }
        }
        // compute sum(count^2)
        long long ss = 0;
        for (int p = 0; p < pnum; ++p) ss += (long long)pcounts[p] * (long long)pcounts[p];
        score_sum = (double)ss / (double)count;

        // cleanup
        for (int p = 0; p < pnum; ++p) free(patterns[p]);
        free(patterns);
        free(pcounts);

        if (score_sum < best_score) {
            best_score = score_sum;
            best_idx = gi;
        }
    }
    return best_idx;
}

static char **find_and_load_wordlist(const char **paths, int npaths, int *out_count) {
    for (int i = 0; i < npaths; ++i) {
        printf("Trying to load: %s\n", paths[i]);
        char **wl = load_wordlist_path(paths[i], out_count);
        if (wl && *out_count > 0) {
            printf("✓ Successfully loaded %d words from: %s\n", *out_count, paths[i]);
            return wl;
        }
        if (wl) free_wordlist(wl, *out_count);
    }
    *out_count = 0;
    return NULL;
}

void solve_with_secret(const char *secret_in, const char *wordlist_path) {
    if (!secret_in || (int)strlen(secret_in) != WORD_LEN) {
        fprintf(stderr, "solve_with_secret: secret must be %d letters\n", WORD_LEN);
        return;
    }
    char secret[WORD_LEN+1];
    strncpy(secret, secret_in, WORD_LEN+1);
    strlower(secret);

    const char *initial_guesses[3] = {"plumb", "crane", "sight"};
    
    const char *default_paths[] = {
        "data/words.txt",
        "./data/words.txt",
        "../data/words.txt",
        "words.txt",
        "./words.txt",
        wordlist_path ? wordlist_path : "",
        "word.txt",
        "./word.txt"
    };

    int total_words = 0;
    char **candidates = find_and_load_wordlist(default_paths, sizeof(default_paths)/sizeof(default_paths[0]), &total_words);
    if (!candidates || total_words == 0) {
        fprintf(stderr, "ERROR: No wordlist loaded. Please ensure data/words.txt exists.\n");
        return;
    }

    printf("\n=== Solver (automatic) ===\n");
    printf("Secret: '%s'\n", secret);
    printf("Wordlist: %d candidates\n\n", total_words);

    char fb[WORD_LEN+1];
    int attempts = 0;

    // First three fixed guesses
    for (int i = 0; i < 3 && attempts < MAX_ATTEMPTS; ++i) {
        const char *g = initial_guesses[i];
        get_feedback(secret, g, fb);
        printf("Attempt %d: %s -> %s (remaining: %d)\n", attempts+1, g, fb, total_words);
        attempts++;
        if (strcmp(fb, "GGGGG") == 0) {
            printf("\n🎉 Solved in %d attempts!\n", attempts);
            free_wordlist(candidates, total_words);
            return;
        }
        // filter candidates
        total_words = filter_candidates_inplace(candidates, total_words, g, fb);
        if (total_words == 0) break;
    }

    // Subsequent attempts: pick best guess among remaining candidates
    while (attempts < MAX_ATTEMPTS && total_words > 0) {
        int pick_idx = pick_best_guess_index(candidates, total_words);
        const char *guess = candidates[pick_idx];
        get_feedback(secret, guess, fb);
        printf("Attempt %d: %s -> %s (remaining: %d)\n", attempts+1, guess, fb, total_words);
        attempts++;
        if (strcmp(fb, "GGGGG") == 0) {
            printf("\n🎉 Solved in %d attempts!\n", attempts);
            free_wordlist(candidates, total_words);
            return;
        }
        total_words = filter_candidates_inplace(candidates, total_words, guess, fb);
        if (total_words == 0) break;
    }

    if (total_words == 0) {
        printf("\n❌ No candidates remain. Secret likely not in wordlist or inconsistent feedback.\n");
    } else {
        printf("\n❌ Attempts exhausted (%d). Remaining candidates: %d. First candidate: %s\n",
               attempts, total_words, candidates[0]);
    }

    free_wordlist(candidates, total_words);
}

void solve_interactive(const char *wordlist_path) {
    const char *default_paths[] = {
        "data/words.txt",
        "./data/words.txt",
        "../data/words.txt",
        "words.txt",
        "./words.txt",
        wordlist_path ? wordlist_path : "",
        "word.txt",
        "./word.txt"
    };

    int total_words = 0;
    char **candidates = find_and_load_wordlist(default_paths, sizeof(default_paths)/sizeof(default_paths[0]), &total_words);
    if (!candidates || total_words == 0) {
        fprintf(stderr, "ERROR: No wordlist loaded. Please ensure data/words.txt exists.\n");
        return;
    }

    printf("\n=== Solver (Interactive Mode) ===\n");
    printf("Wordlist loaded: %d candidates\n", total_words);
    printf("Feedback format: G (green), Y (yellow), B (black/gray)\n");
    printf("Example: GYBBN\n\n");

    const char *initial_guesses[3] = {"plumb", "crane", "sight"};
    char fb_input[256];
    char fb[WORD_LEN+1];
    int attempts = 0;

    for (int i = 0; i < 3 && attempts < MAX_ATTEMPTS; ++i) {
        const char *g = initial_guesses[i];
        printf("=== Attempt %d/%d ===\n", attempts+1, MAX_ATTEMPTS);
        printf("Try this word: %s\n", g);
        printf("Enter feedback (G/Y/B): ");
        
        if (!fgets(fb_input, sizeof(fb_input), stdin)) break;
        
        // Extract and normalize feedback
        int fb_pos = 0;
        for (int j = 0; fb_input[j] && fb_pos < WORD_LEN; ++j) {
            char c = (char)toupper((unsigned char)fb_input[j]);
            if (c == 'G' || c == 'Y' || c == 'B' || c == 'N') {
                // Allow 'N' as alias for 'B' (gray/not present)
                fb[fb_pos++] = (c == 'N') ? 'B' : c;
            }
        }
        fb[fb_pos] = '\0';
        
        if (fb_pos != WORD_LEN) { 
            printf("❌ Invalid feedback length (got %d chars, expected 5). Try again.\n\n", fb_pos); 
            --i; 
            continue; 
        }
        
        attempts++;
        if (strcmp(fb, "GGGGG") == 0) {
            printf("\n🎉 Solved in %d attempts!\n", attempts);
            free_wordlist(candidates, total_words);
            return;
        }
        
        total_words = filter_candidates_inplace(candidates, total_words, g, fb);
        printf("Remaining candidates: %d\n\n", total_words);
        
        if (total_words == 0) break;
    }

    while (attempts < MAX_ATTEMPTS && total_words > 0) {
        int pick_idx = pick_best_guess_index(candidates, total_words);
        const char *guess = candidates[pick_idx];
        
        printf("=== Attempt %d/%d ===\n", attempts+1, MAX_ATTEMPTS);
        printf("Try this word: %s\n", guess);
        printf("(Selected from %d candidates)\n", total_words);
        printf("Enter feedback (G/Y/B): ");
        
        if (!fgets(fb_input, sizeof(fb_input), stdin)) break;
        
        int fb_pos = 0;
        for (int j = 0; fb_input[j] && fb_pos < WORD_LEN; ++j) {
            char c = (char)toupper((unsigned char)fb_input[j]);
            if (c == 'G' || c == 'Y' || c == 'B' || c == 'N') {
                fb[fb_pos++] = (c == 'N') ? 'B' : c;
            }
        }
        fb[fb_pos] = '\0';
        
        if (fb_pos != WORD_LEN) { 
            printf("❌ Invalid feedback length (got %d chars, expected 5). Try again.\n\n", fb_pos); 
            continue; 
        }
        
        attempts++;
        if (strcmp(fb, "GGGGG") == 0) {
            printf("\n🎉 Solved in %d attempts!\n", attempts);
            free_wordlist(candidates, total_words);
            return;
        }
        
        total_words = filter_candidates_inplace(candidates, total_words, guess, fb);
        printf("Remaining candidates: %d\n\n", total_words);
        
        if (total_words == 0) break;
    }

    if (total_words == 0) {
        printf("\n❌ No candidates remain. Feedback inconsistent or secret not in wordlist.\n");
    } else {
        printf("\n❌ Attempts exhausted (%d). Remaining candidates: %d\n", attempts, total_words);
        if (total_words <= 10) {
            printf("Remaining words: ");
            for (int i = 0; i < total_words; ++i) printf("%s ", candidates[i]);
            printf("\n");
        }
    }

    free_wordlist(candidates, total_words);
}
void solve_wordle() {
    solve_interactive(NULL);
}

void solve(const char* target, const char* input) {
    char feedback[WORD_LEN + 1];
    get_feedback(target, input, feedback);
    printf("%s\n", feedback);
}