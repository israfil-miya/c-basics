#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main()
{
	printf("\nHello, world!\n");

	char name[50];

	printf("Provide your name: ");
	fgets(name, sizeof(name), stdin);

	// Remove the newline character from the input, replacing it with a null terminator (\0)
	// name[strcspn(name, "\n")] = '\0'; // This is a safer way to remove the newline character
	name[strlen(name) - 1] = '\0'; // This is a less safer way but works in most cases

	printf("\nOh, hey %s!\n\n", name);

	exit(0);
}