#include <stdlib.h>
#include <stdio.h>

//Problem 2
int main(int argc, char** argv) 
{
	srand(time(NULL));
	printf("Guess your number game!  Think of a number!\n");
	
	// Initial guesses for their number.
	int initial_guesses[] = {4, 7, 10, 13, 50};
	// Steps to take if wrong. Weighted toward moving 1.
	int steps[] = {1, 1, 1, 1, 1, 2, 2, 2, 4, 5};

	int guess = initial_guesses[rand() % 5];
	char answer[1000];
	do {
		printf("\n...is your number %d?\n", guess);
		printf("Am I too (R)ight, (H)igh, or (L)ow?\n");
		printf("Enter R, H, or L: ");
		gets(answer);

		// Change guess
		int increment = steps[rand() % 10];
		if(answer[0] == 'H') increment *= -1;
		guess += increment;
	} while(answer[0] != 'R');
	printf("\n\nThat was fun! Thanks!\n");
	return 0;
}
