#include <stdio.h>
#include "../include/wordle_solver.h"
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>



 void giveRatingToWords(char** wordList,int* scoreList){// gives each word the sum of each letter ferquencies which i called Rating
    for(int i=0;i<wordCount;i++) scoreList[i]=0;
 for (int wordIndex=0;wordIndex<wordCount;wordIndex++){
    for(int letterIndex=0;letterIndex<5;letterIndex++){
      scoreList[wordIndex]+= letterFrequencies[wordList[wordIndex][letterIndex]];
        
    }
 }

 }
  
  void doStats(char** wordList ,int* letterFrequencies,int wordCount){ // Calculate frequencies such as letterfrequencies[0] holds frequency of 'a'
    for (int j=0;j<=25;j++ )letterFrequencies[j]=0;
    for (int i=0;i < wordCount;i++){
        for (int k=0;k<5;k++){
            letterFrequencies[(wordList[i][k]-'a')]++;
        }
    }
  }


  void deleteNonMatchingWords(Feedback feedback,char** wordList) {
    for (int i = 0; i < wordCount; i++) {
        if (!IsWordMatchesFeedback(wordList[i], feedback)) {
            free(wordList[i]);
            wordList[i] = NULL; // Mark as deleted
        }
    }
  }


  void cleanUpWordList(char** wordList) {
    int writeIndex = 0;
    for (int readIndex = 0; readIndex < wordCount; readIndex++) {
        if (wordList[readIndex] != NULL) {
            wordList[writeIndex] = wordList[readIndex];
            writeIndex++;
        }
    }
    // Set remaining entries to NULL
    for (int i = writeIndex; i < wordCount; i++) {
        wordList[i] = NULL;
    }
    wordCount = writeIndex; // Update word count
  }

  void loadWordsToRAM(FILE* src,char** wordList) {
    char buffer[7]; // 5 letters + newline + null terminator
    int index = 0;
    while (fgets(buffer, 7, src) != NULL) {
        wordList[index] = (char*)malloc(7 * sizeof(char));
        if (wordList[index] != NULL) {
            strcpy(wordList[index], buffer);
            index++;
        }
    }
    wordCount = index; // set the global word count
    
  }

  bool greenCompatible(const char* word, LetterFeedback Lfeedback) {
    if (isletterInPosition(Lfeedback.letter, Lfeedback.positions[0], word)) {
        return true;
    }
    return false;
  }
  bool grayCompatible(const char* word, LetterFeedback Lfeedback) {
    if (!isLetterInWord(Lfeedback.letter, word)) {
        return true;
    }
    return false;
  }

  bool yellowCompatible(const char* word, LetterFeedback Lfeedback) {
    if (isLetterInWord(Lfeedback.letter, word)) {
        for (int i = 0; Lfeedback.positions[i] != -1; i++) {
            if (isletterInPosition(Lfeedback.letter, Lfeedback.positions[i], word)) {
                return false;
            }
        }
        return true;
    }
    return false;
  }

  bool IsWordMatchesFeedback(const char* word,Feedback feedback){
    LetterFeedbackNode* current = feedback.head;
    while (current != NULL) {
        if (current->data.feedback == 2) { // green
            if (!greenCompatible(word, current->data)) {
                return false;
            }
        } else if (current->data.feedback == 1) { // yellow
            if (!yellowCompatible(word, current->data)) {
                return false;
            }
        } else if (current->data.feedback == 0) { // gray
            if (!grayCompatible(word, current->data)) {
                return false;
            }
        }
        current = current-> next;
    }
    return true;
    
  }



   

   
   char* getFirstguess(){  //check for errors later
    // copy original words.txt to temp1.txt 

      FILE *src = fopen("../data/words.txt", "r");
      loadWordsToRAM(src,wordList);
        fclose(src);
      
    // pick a random word from a set of best starting words
        srand(time(NULL));
        int randomIndex = rand()%14 + 1; // Generate a random index between 1 and 5
        switch(randomIndex) {
    case 1:
    case 6:
    case 8:
    case 11:
    case 12:
        return "orate";   
    case 2:
    case 7:
    case 14:
        return "soare";   
    case 3:
    case 9:
    case 13:
        return "raise";
    case 4:
    case 10:
        return "arise";
    case 5:
        return "slate";
    default:
        return "orate";   
}
    }

    
   char* getBestGuess(Feedback feedback, int guessNumber){
   
    deleteNonMatchingWords(feedback, wordList);
    cleanUpWordList(wordList);
    letterFrequencies = malloc(sizeof(int) * 26);
    doStats(wordList, letterFrequencies,wordCount);
    scoreList = malloc(wordCount*sizeof(int));
    giveRatingToWords(wordList, scoreList);
   return pick(scoreList,wordList);

    

   }


   
