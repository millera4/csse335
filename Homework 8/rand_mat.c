#include<stdio.h>
#include<stdlib.h>
#include "solve_system_helpers.h"

int main(int argc,char** argv){

  if (argc<4){
    fprintf(stderr,"Usage:\nrand_mat m n outputfile\n");
    exit(0);
  }

  matrix A=random_matrix(atoi(argv[1]),atoi(argv[2]),0,100);
  write_matrix(argv[3],A);

  return 0;
}
