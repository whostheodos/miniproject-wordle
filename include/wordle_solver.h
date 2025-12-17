#ifndef WORDLE_SOLVER_H
#define WORDLE_SOLVER_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_WORDS 2316
#define WORD_LENGTH 5
#define GUESSES 6

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define GRAY "\033[0;90m"
#define RESET "\033[0m"

typedef struct {
    char letter;
    int color;     // 0 = gray, 1 = green, 2 = yellow
    int position;
} LetterFeedback;

/* ===== globals ===== */
extern char** wordList;
extern int wordCount;
extern int* scoreList;
extern int* topScoreWords;
extern int topScoreCount;
extern char target[WORD_LENGTH+1];
extern char guess[WORD_LENGTH+1];
extern LetterFeedback* feedback;
extern bool won;

/* ===== core ===== */
void solve(void);  // NEW function for solver mode

void loadWordsToRAM(FILE* src);
void Touppercase(char* s);
LetterFeedback* Try(const char* word);
bool isInTarget(char c);

/* ===== filtering ===== */
void deleteNonMatchingWords(void);
void cleanUpWordList(void);
bool IsWordMatchesFeedback(const char* word);

bool greenCompatible(const char* word, LetterFeedback f);
bool yellowCompatible(const char* word, LetterFeedback f);
bool grayCompatible(const char* word, LetterFeedback f);
bool isLetterInWord(char c, const char* w);

/* ===== scoring ===== */
void doStats(void);
void giveRatingToWords(void);
int topScore(void);
void getTopScoreWords(void);
char* pick(void);

/* ===== guesses ===== */
char* getFirstGuess(void);
char* getBestGuess(void);

#endif
