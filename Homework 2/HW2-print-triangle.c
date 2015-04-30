#include <stdlib.h>
#include <stdio.h>

int main() {
	int N;
	printf("Enter an N: ");
	scanf("%d", &N);

	int row;
	int col;
	printf("\n");
	for(row = 0; row < N; row++) {
		for(col = 0; col < N; col++) {
			if(col < row)
				printf(" ");
			else
				printf("*");
		}
		printf("\n");
	}
}