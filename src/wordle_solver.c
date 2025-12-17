#include "../include/wordle_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

/* ===== globals ===== */
char** wordList;
int wordCount;

char target[WORD_LENGTH+1];
char guess[WORD_LENGTH+1];

LetterFeedback* feedback = NULL;

int letterFrequencies[26];
int letterFreqPos[26][WORD_LENGTH];

int* scoreList = NULL;
int* topScoreWords = NULL;
int topScoreCount = 0;

bool won = false;

/* ================= CORE ================= */
void Touppercase(char* s)
{
    for (int i = 0; s[i] != '\0'; i++)
        s[i] = toupper((unsigned char)s[i]);
}

LetterFeedback* Try(const char* word)
{
    LetterFeedback* fb = malloc(sizeof(LetterFeedback) * WORD_LENGTH);
    if (feedback != NULL) free(feedback);
    feedback = fb;
    won = true;

    for (int i = 0; i < WORD_LENGTH; i++)
    {
        fb[i].letter = word[i];
        fb[i].position = i;

        if (word[i] == target[i])
        {
            fb[i].color = 1;
            printf("%s%c%s", GREEN, word[i], RESET);
        }
        else
        {
            bool found = false;
            for (int j = 0; j < WORD_LENGTH; j++)
            {
                if (word[i] == target[j])
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                fb[i].color = 2;
                printf("%s%c%s", YELLOW, word[i], RESET);
                won = false;
            }
            else
            {
                fb[i].color = 0;
                printf("%s%c%s", GRAY, word[i], RESET);
                won = false;
            }
        }
    }
    return fb;
}

/* ================= WORD LIST ================= */
void loadWordsToRAM(FILE* src)
{
    char buf[WORD_LENGTH + 2];
    wordCount = 0;

    while (fgets(buf, sizeof(buf), src) && wordCount < MAX_WORDS)
    {
        size_t len = strlen(buf);
        if (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[len - 1] = '\0';

        if (strlen(buf) != WORD_LENGTH) continue;

        wordList[wordCount] = malloc(WORD_LENGTH + 1);
        strcpy(wordList[wordCount], buf);
        Touppercase(wordList[wordCount]);
        wordCount++;
    }
}

/* ================= FILTERING ================= */
bool greenCompatible(const char* w, LetterFeedback f)
{
    return w[f.position] == f.letter;
}

bool yellowCompatible(const char* w, LetterFeedback f)
{
    bool found = false;
    for (int i = 0; i < WORD_LENGTH; i++)
        if (w[i] == f.letter)
            found = true;

    return found && w[f.position] != f.letter;
}

bool grayCompatible(const char* w, LetterFeedback f)
{
    for (int i = 0; i < WORD_LENGTH; i++)
        if (w[i] == f.letter)
            return false;
    return true;
}

bool IsWordMatchesFeedback(const char* word)
{
    for (int i = 0; i < WORD_LENGTH; i++)
    {
        if (feedback[i].color == 1 && !greenCompatible(word, feedback[i])) return false;
        if (feedback[i].color == 2 && !yellowCompatible(word, feedback[i])) return false;
        if (feedback[i].color == 0 && !grayCompatible(word, feedback[i])) return false;
    }
    return true;
}

void deleteNonMatchingWords(void)
{
    for (int i = 0; i < wordCount; i++)
    {
        if (!IsWordMatchesFeedback(wordList[i]))
        {
            free(wordList[i]);
            wordList[i] = NULL;
        }
    }
}

void cleanUpWordList(void)
{
    int w = 0;
    for (int i = 0; i < wordCount; i++)
        if (wordList[i])
            wordList[w++] = wordList[i];

    wordCount = w;
}

/* ================= SCORING ================= */
void doStats(void)
{
    memset(letterFrequencies, 0, sizeof(letterFrequencies));
    memset(letterFreqPos, 0, sizeof(letterFreqPos));

    for (int i = 0; i < wordCount; i++)
    {
        bool seen[26] = {0};
        for (int p = 0; p < WORD_LENGTH; p++)
        {
            int idx = wordList[i][p] - 'A';
            letterFreqPos[idx][p]++;
            if (!seen[idx])
            {
                letterFrequencies[idx]++;
                seen[idx] = true;
            }
        }
    }
}

void giveRatingToWords(void)
{
    if (!scoreList)
        scoreList = malloc(sizeof(int) * wordCount);

    for (int w = 0; w < wordCount; w++)
    {
        int score = 0;
        bool seen[26] = {0};

        for (int p = 0; p < WORD_LENGTH; p++)
        {
            int idx = wordList[w][p] - 'A';
            score += letterFrequencies[idx];
            score += letterFreqPos[idx][p] * 2;
            if (seen[idx]) score -= 8;
            else seen[idx] = true;
        }
        scoreList[w] = score;
    }
}

int topScore(void)
{
    int max = scoreList[0];
    topScoreCount = 1;

    for (int i = 1; i < wordCount; i++)
    {
        if (scoreList[i] > max)
        {
            max = scoreList[i];
            topScoreCount = 1;
        }
        else if (scoreList[i] == max)
            topScoreCount++;
    }
    return max;
}

void getTopScoreWords(void)
{
    int max = topScore();
    free(topScoreWords);
    topScoreWords = malloc(sizeof(int) * topScoreCount);

    int c = 0;
    for (int i = 0; i < wordCount; i++)
        if (scoreList[i] == max)
            topScoreWords[c++] = i;
}

char* pick(void)
{
    return wordList[topScoreWords[rand() % topScoreCount]];
}

/* ================= GUESSES ================= */
char* getFirstGuess(void)
{
    static char* starters[] = {"ORATE","SOARE","RAISE","ARISE","SLATE"};
    return starters[rand() % 5];
}

char* getBestGuess(void)
{
    deleteNonMatchingWords();
    cleanUpWordList();
    doStats();
    giveRatingToWords();
    getTopScoreWords();
    return pick();
}

/* ================= SOLVE ================= */
void solve(void)
{
    srand((unsigned int)time(NULL));

    FILE* src = fopen("data/words.txt", "r");
    if (!src)
    {
        printf("Cannot open data/words.txt\n");
        return;
    }

    wordList = malloc(sizeof(char*) * MAX_WORDS);
    loadWordsToRAM(src);
    fclose(src);

    strcpy(target, wordList[rand() % wordCount]);
    printf("Target (hidden): %s\n\n", target);

    for (int turn = 1; turn <= GUESSES; turn++)
    {
        char* g = (turn == 1) ? getFirstGuess() : getBestGuess();
        strcpy(guess, g);
        Touppercase(guess);
        Try(guess);
        printf("\n");

        if (won)
        {
            printf("Solved in %d guesses!\n", turn);
            break;
        }
    }

    for (int i = 0; i < wordCount; i++) free(wordList[i]);
    free(wordList);
    free(feedback);
    free(scoreList);
    free(topScoreWords);
}
