// Enum to represent the days of the week
typedef enum WeekDay // can be unnamed but it's better to name it for clarity
{
    Sunday = 1,
    Monday = 2,
    Tuesday = 3,
    Wednesday = 4,
    Thursday = 5,
    Friday = 6,
    Saturday = 7
} weekday;

// Struct to represent a day of the week
typedef struct Day // can be unnamed but it's better to name it for clarity
{
    weekday day_index;
} day;
