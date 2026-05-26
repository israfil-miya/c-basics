#include <stdio.h>
#include <stdlib.h>

// function prototype
void print_array(int *p, int size);

int main()
{
    int numbers[10] = {13, 23, 34, 45, 56, 67, 78, 89, 90, 100};
    int *p = numbers;

    printf("The numbers are: ");

    // sizeof(array) / sizeof(one of the elements of the array) = calculate the number of elements in the array
    print_array(p, sizeof(numbers) / sizeof(numbers[0]));

    return 0;
}

void print_array(int *p, int size)
{
    if (size == 0)
    {
        return;
    }
    printf("%d ", *p);
    print_array(p + 1, size - 1);
}