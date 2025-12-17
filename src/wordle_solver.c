#include "../include/wordle_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

// globals
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

int won = 0;

/* ================= CORE ================= */
void Touppercase(char* s)
{
    int i;
    for(i=0; s[i]!='\0'; i++)
        s[i] = toupper((unsigned char)s[i]);
}

LetterFeedback* Try(const char* word)
{
    int i,j;
    LetterFeedback* fb = (LetterFeedback*)malloc(sizeof(LetterFeedback)*WORD_LENGTH);
    if(feedback!=NULL) free(feedback);
    feedback = fb;
    won = 1;

    for(i=0; i<WORD_LENGTH; i++)
    {
        fb[i].letter = word[i];
        fb[i].position = i;

        if(word[i]==target[i])
        {
            fb[i].color = 1;
            printf("%s%c%s", GREEN, word[i], RESET);
        }
        else
        {
            // check if letter is in target
            int found = 0;
            for(j=0; j<WORD_LENGTH; j++)
            {
                if(word[i]==target[j])
                {
                    found=1;
                    break;
                }
            }
            if(found)
            {
                fb[i].color = 2;
                printf("%s%c%s", YELLOW, word[i], RESET);
                won = 0;
            }
            else
            {
                fb[i].color = 0;
                printf("%s%c%s", GRAY, word[i], RESET);
                won = 0;
            }
        }
    }
    return fb;
}

// loading words
void loadWordsToRAM(FILE* src)
{
    char buf[WORD_LENGTH+2];
    wordCount = 0;
    while(fgets(buf,sizeof(buf),src)!=NULL && wordCount<MAX_WORDS)
    {
        int len = strlen(buf);
        if(len>0 && (buf[len-1]=='\n' || buf[len-1]=='\r')) buf[len-1]=0;
        if(strlen(buf)!=WORD_LENGTH) continue;
        wordList[wordCount] = (char*)malloc(WORD_LENGTH+1);
        strcpy(wordList[wordCount],buf);
        Touppercase(wordList[wordCount]);
        wordCount++;
    }
}
// filtring
int greenCompatible(const char* w, LetterFeedback f)
{
    return w[f.position]==f.letter;
}

int yellowCompatible(const char* w, LetterFeedback f)
{
    int i, found=0;
    for(i=0;i<WORD_LENGTH;i++)
    {
        if(w[i]==f.letter) found=1;
    }
    if(found && w[f.position]!=f.letter) return 1;
    return 0;
}

int grayCompatible(const char* w, LetterFeedback f)
{
    int i;
    for(i=0;i<WORD_LENGTH;i++)
        if(w[i]==f.letter) return 0;
    return 1;
}

int IsWordMatchesFeedback(const char* word)
{
    int i;
    for(i=0;i<WORD_LENGTH;i++)
    {
        if(feedback[i].color==1 && !greenCompatible(word,feedback[i])) return 0;
        if(feedback[i].color==2 && !yellowCompatible(word,feedback[i])) return 0;
        if(feedback[i].color==0 && !grayCompatible(word,feedback[i])) return 0;
    }
    return 1;
}

void deleteNonMatchingWords(void)
{
    int i;
    for(i=0;i<wordCount;i++)
    {
        if(!IsWordMatchesFeedback(wordList[i]))
        {
            free(wordList[i]);
            wordList[i]=NULL;
        }
    }
}

void cleanUpWordList(void)
{
    int i,w;
    w=0;
    for(i=0;i<wordCount;i++)
    {
        if(wordList[i]!=NULL)
        {
            wordList[w]=wordList[i];
            w++;
        }
    }
    wordCount=w;
}

//scoring words
void doStats(void)
{
    int i,p;
    for(i=0;i<26;i++)
        letterFrequencies[i]=0;
    for(i=0;i<26;i++)
        for(p=0;p<WORD_LENGTH;p++)
            letterFreqPos[i][p]=0;

    for(i=0;i<wordCount;i++)
    {
        int seen[26];
        for(p=0;p<26;p++) seen[p]=0;
        int pos;
        for(pos=0;pos<WORD_LENGTH;pos++)
        {
            int idx = wordList[i][pos]-'A';
            letterFreqPos[idx][pos]++;
            if(!seen[idx])
            {
                letterFrequencies[idx]++;
                seen[idx]=1;
            }
        }
    }
}

void giveRatingToWords(void)
{
    int w,p;
    if(scoreList==NULL) scoreList = (int*)malloc(sizeof(int)*wordCount);
    for(w=0;w<wordCount;w++)
    {
        int score=0;
        int seen[26];
        for(p=0;p<26;p++) seen[p]=0;
        for(p=0;p<WORD_LENGTH;p++)
        {
            int idx = wordList[w][p]-'A';
            score += letterFrequencies[idx];
            score += letterFreqPos[idx][p]*2;
            if(seen[idx]) score -= 8;
            else seen[idx]=1;
        }
        scoreList[w]=score;
    }
}

int topScore(void)
{
    int i,max, count;
    max = scoreList[0];
    topScoreCount=1;
    for(i=1;i<wordCount;i++)
    {
        if(scoreList[i]>max)
        {
            max = scoreList[i];
            topScoreCount = 1;
        }
        else if(scoreList[i]==max) topScoreCount++;
    }
    return max;
}

void getTopScoreWords(void)
{
    int i,c,max;
    max = topScore();
    if(topScoreWords!=NULL) free(topScoreWords);
    topScoreWords = (int*)malloc(sizeof(int)*topScoreCount);
    c=0;
    for(i=0;i<wordCount;i++)
        if(scoreList[i]==max)
        {
            topScoreWords[c]=i;
            c++;
        }
}

char* pick(void)
{
    int r;
    r = rand()%topScoreCount;
    return wordList[topScoreWords[r]];
}

//getting guess
char* getFirstGuess(void)
{
    int r;
    char* starters[]={"ORATE","SOARE","RAISE","ARISE","SLATE"};
    r = rand()%5;
    return starters[r];
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
// solve function
void solve(void)
{
    int turn;
    FILE* src;
    srand((unsigned int)time(NULL));

    src = fopen("data/words.txt","r");
    if(src==NULL)
    {
        printf("Cannot open data/words.txt\n");
        return;
    }

    wordList = (char**)malloc(sizeof(char*)*MAX_WORDS);
    loadWordsToRAM(src);
    fclose(src);

    turn = 1;
    strcpy(target, wordList[rand()%wordCount]);
    Touppercase(target);
    printf("Target (hidden): %s\n\n", target);

    while(turn<=GUESSES)
    {
        char* g;
        if(turn==1)
            g = getFirstGuess();
        else
            g = getBestGuess();

        strcpy(guess,g);
        Touppercase(guess);
        Try(guess);
        printf("\n");
        if(won)
        {
            printf("Solved in %d guesses!\n", turn);
            break;
        }
        turn++;
    }

    //free stuff
    for(turn=0;turn<wordCount;turn++) free(wordList[turn]);
    free(wordList);
    free(feedback);
    free(scoreList);
    free(topScoreWords);
}
