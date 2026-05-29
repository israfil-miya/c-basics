#include <stdio.h>
#include "include/factorial_recursion.h"

int main()
{
    int factorial_of;

    printf("Enter a number to calculate its factorial: ");
    scanf("%d", &factorial_of);

    if (factorial_of < 0)
    {
        printf("Factorial can't be calculated for negative numbers.\n");
        return 1;
    }

    long long int factorial = fact(factorial_of);

    printf("%d! = %lld\n", factorial_of, factorial);

    return 0;
}
