#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int main() {
    const int MIN = 0;
    const int MAX = 100;

    int guess, guesses, answer;
    char response[4];

    srand(time(0));
    answer = (rand() % MAX) + MIN; // random number between MIN and MAX

    printf("Guess a number between %d and %d: ", MIN, MAX);
    do {
        scanf("%d", &guess);
        strcpy(response, guess > answer ? "high" : "low");
        if (guess != answer) {
            printf("Wrong, that number is too %s!\n", response);
            printf("Guess again: ");
        }

        guesses++;

    } while (guess != answer);


    printf("\nCORRECT, WELL DONE!\n");
    printf("Guesses: %d\n", guesses);
    printf("Answer: %d\n", answer);

    return 0;
}