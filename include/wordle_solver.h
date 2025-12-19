#ifndef WORDLE_SOLVER_H
#define WORDLE_SOLVER_H

#define GUESSES 6
#define WORD_LENGTH 5
#define MAX_WORDS 2316



#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define GRAY "\033[0;90m"
#define RESET "\033[0m"

typedef struct {
    char letter;
    int position;
    int color;
} LetterFeedback;

void solve();

#endif
