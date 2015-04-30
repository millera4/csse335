#include <stdlib.h>
#include <stdio.h>

// Problem 3, 4, 5
int main() {
	int pancakes[10];
	int people[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	int i;

	for(i = 0; i < 10; i++) {
		printf("Enter the number of pancakes person %d ate: ", i+1);
		scanf("%d", pancakes+i);
	}

	// Analyze after input

	/* Problem 3 and 4, find min and max
	int max = pancakes[0];
	int max_index = 0;

	int min = pancakes[0];
	int min_index = 0;

	for(i = 1; i < 10; i++) {
		if (pancakes[i] > max) {
			max = pancakes[i];
			max_index = i;
		} else if (pancakes[i] < min) {
			min = pancakes[i];
			min_index = i;
		}
	}
	*/

	// Problem 5, sorting. (baby's first sort)
	int j;
	int val, people_val;
	for(i = 1; i < 10; i++) {
		val = pancakes[i];
		people_val = people[i];
		j = i;
		while(j > 0) {
			if(pancakes[j - 1] > val) {
				pancakes[j] = pancakes[j - 1];
				people[j] = people[j - 1];
				j--;
			} else {
				break;
			}
		}
		pancakes[j] = val;
		people[j] = people_val;
	}

	printf("Person %d ate the most pancakes.  They ate %d pancakes!  Wow! \n", people[9], pancakes[9]);
	printf("Person %d ate the least pancakes.  They only ate %d pancakes!  Boo! \n", people[0], pancakes[0]);

	// Print in order
	printf("In order from least to greatest, the number of pancakes eaten is:\n");
	for(i = 0; i < 10; i++) {
		printf("\tPerson %d ate %d pancakes\n", people[i], pancakes[i]);
	}
}
