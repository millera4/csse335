//Write a program that asks the user to type the value of N and computes N! .

#include <stdio.h>

int nfact(int n);
 
int main()
{
 int N,i,f=1;
 
 printf("Type the value of N : ");
 scanf("%d",&N); 
 printf("%d!  is %d\n",N,nfact(N));
 return 0;
}


int nfact(int n) {
  int i;
  int fact=1;
  for (i=2;i<=n;i++){
    fact*=i;
  }
  return fact;
}
