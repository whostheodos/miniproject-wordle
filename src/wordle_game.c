#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "../include/wordle_game.h"

#include <stdlib.h>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define GRAY "\033[0;90m"
#define RESET "\033[0m"
#define MAX_WORDS 100 //max to load

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
    const char* target_word = get_random_word();
    printf("------- WORDLE GAME -------\n");
    printf("guess the 5-letter word. You have %d attempts.\n", 5);
    printf("green = correct letter in correct position\n");
    printf("yellow = correct letter in wrong position\n");
    printf("gray = letter not in word\n\n");

    char input[100];
    int attempts = 0;
    int won = 0;

    while (attempts < 5 && !won) {
        printf("");
    }
}