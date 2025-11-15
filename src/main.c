#include <stdio.h>
#include "../include/wordle_game.h"

int main() {
    int index = 0;

    printf("Enter 1 to play Wordle: ");
    scanf("%d", &index);


    if (index == 1) {
        play();
    } else {
        printf("Invalid option. Exiting.\n");
    }

    return 0;
}