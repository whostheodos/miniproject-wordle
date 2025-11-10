#ifndef WORDLE_GAME_H
#define WORDLE_GAME_H

void play();

void evaluate(const char* target,const char* input);
void uppercase(char* str);
const char* get_random_word();




#endif