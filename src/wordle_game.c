#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "../include/wordle_game.h"
#include <stdlib.h>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define GRAY "\033[0;90m"
#define RESET "\033[0m"

#define MAX_WORDS 100 //max to load
#define WORD_LENGTH 5

char word_list[MAX_WORDS][WORD_LENGTH + 1];
int word_count = 0;

int load_words(const char* filename) {
    FILE* file =fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file '%s'\n", filename);
        printf("Make sure the file exists in the same directory as the program.\n");
        return 0;
    }
    char buffer[100];
    word_count = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && word_count < MAX_WORDS) {

        buffer[strcspn(buffer, "\r\n")] = 0;


        if (strlen(buffer) == 0) continue;


        for (int i = 0; buffer[i]; i++) {
            buffer[i] = toupper(buffer[i]);
        }


        if (strlen(buffer) == WORD_LENGTH) {
            strcpy(word_list[word_count], buffer);
            word_count++;
        }
    }

    fclose(file);
    if (word_count == 0) {
        printf("Error: No valid 5-letter words found in file\n");
        return 0;
    }

    return 1;

}
const char* get_random_word() {
    if (word_count == 0) {
        return NULL;
    }
    return word_list[rand() % word_count];
}


void uppercase(char* str) {
    for (int i = 0; str[i];i++) {
    str[i] = toupper(str[i]);
    }
}

int is_valid_word(const char* word) {
    if (strlen(word)!=5) return 0;
    for (int i = 0;i < word_length ;i++) {
        if (!isalpha(word[i])) return 0;
    }
    return 1;
}

void evaluate(const char* target,const char* input) {
    char result[word_length];
    int target_count[26] = {0};
    int used[word_length] = {0};

    for (int i = 0; i < word_length; i++) {
        if (input[i] == target[i]) {
            result[i] = 'G';
            used[i] = 1;
        }else {
            target_count[input[i] - 'A']++;
        }
    }
    for (int i = 0; i < word_length; i++) {
        if (result[i] == 'G') continue;
        if (target_count[input[i] - 'A'] > 0) {
            result[i] = 'Y';
            target_count[input[i] - 'A']--;
        }else {
            result[i] = 'N';
        }
    }
    for (int i = 0; i < word_length; i++) {
        if (result[i] == 'G') {
            printf("%s%c%s ",GREEN,input[i],RESET);
        }else if (result[i] == 'Y') {
            printf("%s%c%s ",YELLOW,input[i],RESET);
        }else if (result[i] == 'N') {
            printf("%s%c%s ",GRAY,input[i],RESET);
        }
    }
    printf("\n");
}

void play() {
    printf("------- WORDLE GAME -------\n");
    printf("guess the 5-letter word. You have %d attempts.\n", 5);
    printf("green = correct letter in correct position\n");
    printf("yellow = correct letter in wrong position\n");
    printf("gray = letter not in word\n\n");

    char input[100];
    int attempts = 0;
    int won = 0;

    load_words("data/words.txt");

    srand(time(NULL));






    const char* target_word = get_random_word();
    if (target_word == NULL) {
        printf("no words available");
        return;
    }
    while (attempts < 5 && !won) {
        printf("Attempt %d/%d: ", attempts + 1, 5);
        if (fgets(input, sizeof(input), stdin) == NULL) {
        }

        input[strcspn(input, "\n")] = 0;

        uppercase(input);

        // Validate input
        if (!is_valid_word(input) ){
            printf("invalid word\n");
            continue;
        }

        // Evaluate and display result
        evaluate(target_word, input);

        // Check if won
        if (strcmp(input, target_word) == 0) {
            won = 1;
            printf("\n You guessed the word in %d attempts!\n", attempts + 1);
        }

        attempts++;

        if (!won && attempts == 5) {
            printf("game over,the word was %s :)", target_word);
        }
    }

}


