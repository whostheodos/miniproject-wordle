#ifndef WORDLE_SOLVER_H
#define WORDLE_SOLVER_H
#include <stdio.h>
#include <stdbool.h>


typedef struct {
    char letter;
    int feedback; // 0 = absent, 1 = present, 2 = correct
    int* positions; // position of the letter in the word
} LetterFeedback;

typedef struct LetterFeedbackNode {
    LetterFeedback data;
    struct LetterFeedbackNode* next;
} LetterFeedbackNode;

typedef struct {
   LetterFeedbackNode* head;
} Feedback;

char** wordList; // Array of words loaded from words.txt
int wordCount;  // Total number of words loaded
int* letterFrequencies; // Array to hold letter frequencies




char* getFirstguess();//get the best first guess
void deleteNonMatchingWords(Feedback feedback,char** wordList);// delete words that do not match the feedback
void loadWordsToRAM(FILE* src,char** wordList); // load words from words.txt to RAM
char* getBestGuess(Feedback feedback, int guessNumber);//get the best guess based on feedback and guess number
bool isLetterInWord(char letter, const char* word);// check if a letter is in a word
bool isletterInPosition(char letter, int position, const char* word); // check if a letter is in a specific position in a word
bool grayCompatible(const char* word, LetterFeedback Lfeedback); // check if a word is compatible with gray feedback
bool yellowCompatible(const char* word, LetterFeedback Lfeedback); // check if a word is compatible with yellow feedback
bool greenCompatible(const char* word, LetterFeedback Lfeedback); // check if a word is compatible with green feedback
char* pick (FILE* StatsFile); // picks the best word based on stats file
void giveRatingToWords(FILE* source, FILE* destination); // give rating to words based on stats file
bool IsWordMatchesFeedback(const char* word,Feedback feedback); // check if a word is valid by looking it up in words.txt
void cleanUpWordList(char** wordList); // shift the² words to remove NULLs
void doStats(char** wordList ,int* letterFrequencies,int WordCount);// return an array of letter frequencies based on the current word list
#endif 
