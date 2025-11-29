#include <stdio.h>
#include "../include/wordle_game.h"
#include "../include/wordle_solver.h"

int main(){
    int i;
    printf("type 1 to play or 2 to solve");
    scanf("%d",&i);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (i == 1) {
        play();

    }else if (i == 2) {
        solve_wordle();

    }
    return 0;
}