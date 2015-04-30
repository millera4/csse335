#include <stdlib.h>
#include <stdio.h>

int main() {
	int num[10];

	printf("Please enter 10 numbers:\n");
	int i;
	for(i = 0; i < 10; i++) {
		printf("Number %d: ", i+1);
		scanf("%d", num + i);
	}

	// "Ask, then find sum" (could find sum as we ask)
	int sum = 0;
	for(i = 0; i < 10; i++) {
		sum += num[i];
	}

	printf("\nThe sum is %d\n", sum);
}
