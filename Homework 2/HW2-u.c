#include <stdlib.h>
#include <stdio.h>

int u(int n) {
	if(n == 0)
		return 3;
	return 3*u(n-1) + 4;
}

int main() {
	int N;
	printf("Enter an N: ");
	scanf("%d", &N);

	int result = u(N);
	printf("Result: u(%d) = %d\n", N, result);
}
