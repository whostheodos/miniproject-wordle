#include <stdio.h>
#include "../include/wordle_game.h"
#include "../include/wordle_solver.h"

int main(){
    int mode;
    printf("Type 1 to play or 2 to solve: ");
    scanf("%d",&mode);
    int c; while((c=getchar())!='\n' && c!=EOF);

    if(mode == 1){
        play();
    } else if(mode == 2){
        solve();
    } else {
        printf("Invalid option.\n");
    }

    return 0;
}

