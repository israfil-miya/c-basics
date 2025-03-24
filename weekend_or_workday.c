#include <stdio.h>
#include "comps/wd.h"
#include <time.h>

int main() {
    day weekend1 = {Sunday};
    day weekend2 = {Saturday};

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    int today = tm_info->tm_wday + 1; // tm_wday is 0-indexed

    if (today == weekend1.day_index || today == weekend2.day_index) {
        printf("It's the weekend!\n");
    } else {
        printf("It's a work day.\n");
    }
    
    return 0;
}
