#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main() {
	printf("\nHello, world!\n");
	
	char name[50];
	
	printf("Provide your name: ");
	fgets(name, sizeof(name), stdin);
	
	// name[strcspn(name, "\n")] = '\0';
	name[strlen(name) - 1] = '\0';
	
	printf("\nOh, hey %s!\n\n", name);
	
	exit(0);
}