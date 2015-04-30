#include <stdlib.h>
#include <stdio.h>

int countEven(int* arr, int size) {
	int count = 0;
	int i;

	for(i = 0; i < size; i++) {
		if(*(arr + i) % 2 == 0)
			count++;
	}

	return count;
}

double* maximum(double* a, int size) {
	double* max = a;
	int i;

	for(i = 0; i < size; i++) {
		if (*(a + i) > *max) {
			max = a + i;
		}
	}

	return max;
}

void reverseArgs(int argc, char** argv) {
	int i;
	for(i = argc - 1; i >= 0; i--) {
		printf("%s ", *(argv + i));
	}
}

int myStrLen(char* s) {
	int len = 0;
	while( *(s + len) != '\0') 
		len++;
	return len;
}

void revString(char* s) {
	int len = strlen(s); // myStrLen(s)!
	int i;
	for(i = 0; i < len/2; i++) {
		char tmp = *(s + i);
		*(s + i) = *(s + len - i - 1); // Swap s[i] and s[len - i - 1]
		*(s + len - i - 1) = tmp;
	}
}

void delEven(int* arr, int size) {
	for(i = 0; i < size; i++) {
		// An effort to make a difficult to read example
		*(arr + i) = *(arr + i) % 2 == 0 ? -1 : *(arr + i);

		// if (*(arr + i) % 2 == 0) 
		//	 *(arr + i) = -1
	}
}

int main(int argc, char** argv) {
	printf("Command line args: %s, %d, %d", argv[0], argv[1], argv[2]);

	int result = countEven(*(argv+2), *(argv+1));
	printf("The result is: %d\n", result);
}