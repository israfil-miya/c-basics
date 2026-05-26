#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "include/week_day.h"

int main(void)
{
    day weekend1 = {.day_index = Sunday};
    day weekend2 = {.day_index = Saturday};

    time_t t = time(NULL);

    struct tm *tm_info = localtime(&t);

    if (tm_info == NULL)
    {
        fprintf(stderr, "Failed to get local time.\n");
        return 1;
    }

    // tm_wday:
    // 0 = Sunday
    // 1 = Monday
    // ...
    // 6 = Saturday
    weekday today = (weekday)tm_info->tm_wday;

    if (today == weekend1.day_index || today == weekend2.day_index)
    {
        printf("It's the weekend!\n");
    }
    else
    {
        printf("It's a work day.\n");
    }

    return 0;
}
