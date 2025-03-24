#include <stdio.h>
#include "comps/fact.h"
#include "comps/fact.c"

int main() {
    int factorial_of;
    
    scanf("%d", &factorial_of);
    
    if (factorial_of < 0) {
        printf("Factorial can't be calculated for negative numbers.\n");
        return 1;
    }
    
    long long int factorial = fact(factorial_of);
    
    printf("%d! = %lld\n", factorial_of, factorial);
    
    return 0;
}

