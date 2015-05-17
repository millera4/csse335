#include <stdio.h>
#include <stdlib.h>
#include "solve_system_helpers.h"

void guassian_eliminate(matrix* A, matrix* b);
void back_sub(matrix* A, matrix* b, matrix* x);

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
  // do_some_stuff(&A, &b, &x);
  gaussian_eliminate(&A, &b);
  back_sub(&A, &b, &x)

  write_matrix(argv[3],x);
}

void gaussian_eliminate(matrix* A, matrix* b) {
  // do_some_stuff
}

void back_sub(matrix* A, matrix* b, matrix* x) {
  // do_some_stuff
}