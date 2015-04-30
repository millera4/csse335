//Write a program that asks the user to type an integer N and that indicates if N is a prime number or not.

#include <stdio.h>

// NOTE: this program is incorrect. For example, it will say 2 is not prime, 4 is prime.
//  The assignment was not to fix the program logic but to make it compile, so I will leave it as-is.
int isprime(int n){
  int i;
  int prime=1; 
  if (n==2){ 
    prime=0; // 2 is not prime? Incorrect, but still compiles
  }
  else{
    for (i=3;i<n ;i++){
      if(n%i==0){ 
	prime=0;
      }
    } 
  }
  return prime;
}
 
int main(){
 int N;  
 int prime=1; 
 printf("Write a number: ");
 scanf("%d",&N);
 if (isprime(N)) {
     printf("%d is prime\n",N);
 }
 else{
     printf("%d is not prime.\n",N);
 }
 
 return 0;
}
