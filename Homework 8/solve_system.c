#include <stdio.h>
#include <stdlib.h>
#include "solve_system_helpers.h"

void main(int argc,char** argv){

  if (argc<3){
    printf("Usage: solve_system A b x\n");
    printf("Here A is the file storing the matrix A\n");
    printf("b is the file storing the vector b");
    printf("and x is the output file");
    exit(1);
  }

  matrix A=read_matrix(argv[1]);
  matrix b=read_matrix(argv[2]);
  matrix x=new_matrix(b.m,1);

  // do some stuff.

  write_matrix(argv[3],x);
}
