#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "../include/wordle_game.h"


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