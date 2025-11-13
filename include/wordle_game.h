#ifndef WORDLE_GAME_H
#define WORDLE_GAME_H
#define word_length 5

void play();

void evaluate(const char* target,const char* input);
void uppercase(char* str);
const char* get_random_word();
int is_valid_word(const char* word);
int load_words(const char* file_name);
int get_words_count();




#endif