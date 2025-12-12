#ifndef WORDLE_SOLVER_H
#define WORDLE_SOLVER_H
#include <stdio.h>
#include <stdbool.h>


typedef struct {
    char letter;
    int feedback; // 0 = absent, 1 = present, 2 = correct
    int* positions; // position of the letter in the word
} LetterFeedback;

typedef struct  {
    LetterFeedback data;
    struct LetterNode* next;
} LetterNode;

typedef struct {
    LetterFeedback* head;
    LetterFeedback* phaseHead;
} Feedback;

void solve(); // main solver function
char* getbestguess (int guessNumber);//determine the suitable function to get the best guess
char* getBestGuess1();//get the best first guess
char* getBestGuess2(Feedback feedback, int guessNumber);//get the best guess based on feedback and guess number
bool isLetterInWord(char letter, const char* word);// check if a letter is in a word
bool isletterInPosition(char letter, int position, const char* word); // check if a letter is in a specific position in a word
bool grayCompatible(const char* word, LetterFeedback Lfeedback); // check if a word is compatible with gray feedback
bool yellowCompatible(const char* word, LetterFeedback Lfeedback); // check if a word is compatible with yellow feedback
bool greenCompatible(const char* word, LetterFeedback Lfeedback); // check if a word is compatible with green feedback
void keepOnlyInterphaseFeedback(Feedback* feedback);// clears old feedback and keep only interphase one
FILE* doTempStats(FILE* file2 ); //creates a tempfile that has most common letters in words and their occururences
char* pick (FILE* StatsFile); // picks the best word based on stats file
void copyTXTFileContents(FILE* source, FILE* destination); // copy contents of one file to another
void giveRatingToWords(FILE* source, FILE* destination); // give rating to words based on stats file
bool IsWordMatchesFeedback(const char* word,Feedback feedback); // check if a word is valid by looking it up in words.txt

#endif
