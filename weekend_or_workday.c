#include <stdio.h>
#include "comps/wd.h"
#include <time.h>
#include <stdlib.h>

int main() {
    day weekend1 = {Sunday};
    day weekend2 = {Saturday};

    time_t t = time(NULL);
    struct tm* tm_info = NULL;

    tm_info = (struct tm*) malloc(sizeof(struct tm));
    if (tm_info == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

   tm_info = localtime(&t); // localtime() returns a pointer to a static tm structure, so we need to copy it to our allocated memory

    // for pointers to structs, use ( -> ) to access members
    int today = tm_info->tm_wday + 1; // tm_wday is 0-indexed


    if (today == weekend1.day_index || today == weekend2.day_index) {
        printf("It's the weekend!\n");
    } else {
        printf("It's a work day.\n");
    }
    
    return 0;
}