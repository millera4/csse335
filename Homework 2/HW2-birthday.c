#include <stdlib.h>
#include <stdio.h>

//Problem 1
int main(int argc, char** argv) 
{
	int MY_BIRTHDAY = 1995;
	char name[1000];
	int year;

	printf("Enter your name:\n");
	gets(name);
	printf("Enter your birth year:\n");
	scanf("%d", &year);

	if(year < MY_BIRTHDAY)
		printf("Old\n");

	return 0;
}