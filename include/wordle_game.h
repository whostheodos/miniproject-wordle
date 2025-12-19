#ifndef WORDLE_GAME_H
#define WORDLE_GAME_H

void play();

void evaluate(const char* target,const char* input);
void uppercase(char* str);
const char* get_random_word();
int is_valid_word(const char* word);
int load_words(const char* filename);
int get_words_count();




#endif