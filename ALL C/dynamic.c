#include <stdio.h>
#include <stdlib.h>

/*
    Dynamic memory allocation in C

    malloc() - allocate memory
    realloc() - reallocate memory
    free() - free memory

    malloc() and realloc() return NULL if the allocation fails.
    Always check for NULL after calling malloc() or realloc().

    Common rule of thumb:
    1. T* var is a pointer to a variable of type T (returns address of the first byte of the allocated memory)
    2. *var is the variable of type T (dereference (using * before variable name) the pointer to get the value stored in the memory location)
    3. we can do T** var, T*** var etc.
    4. similarly we can do **var, ***var etc.
    5. we can also create pointers to arrays of type T, i.e. T* var is a pointer to the first element of an array of type T, then we need to allocate memory for the array elements using malloc() or realloc()
    6. var[i] is the i-th element of the array (array notation is just syntactic sugar for pointer arithmetic) i.e. var[i] is equivalent to *(var + i)
*/

int main()
{

    int *ages = NULL;
    int n = 4;

    ages = (int *)malloc(n * sizeof(int));

    if (ages == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
    {

        char line[1024];
        printf("Enter age %d: ", i + 1);
        while (fgets(line, sizeof(line), stdin))
        {
            // %i will accept decimal, octal and hexadecimal numbers
            // sscanf will return the number of items successfully read
            // sscanf will return 1 if it successfully read one item
            if (sscanf(line, "%i ", ages + i) != 1)
            {
                fprintf(stderr, "Please enter a number.\n");
                printf("Enter age %d: ", i + 1);
                continue;
            }

            if (*(ages + i) < 0)
            {
                fprintf(stderr, "Please enter a positive number.\n");
                printf("Enter age %d: ", i + 1);
                continue;
            }

            break;
        }
    }

    printf("Ages entered:\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\n", *(ages + i));
    }

    n = 6;

    ages = (int *)realloc(ages, n * sizeof(int));
    ages[4] = 32;
    ages[5] = 59;

    printf("\nAges after realloc:\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\n", *(ages + i));
    }

    free(ages);
}