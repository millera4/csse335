// Only set up to work for square matrices.  A lot of weird stuff can happen with 
// non-square matrices that isn't relevant to being parallel.
#include <stdio.h>
#include <stdlib.h>
#include "solve_system_helpers.h"

void guassian_eliminate(matrix* A, matrix* b);
void back_sub(matrix* A, matrix* b, matrix* x);

// From a matrix M, gets M[row][col]
double get(matrix* M, int row, int col) {
  double* arr = M->data;
  int m = M->m;
  return arr[m*row + col];
}

// As above, but returns a pointer to the location for assignment
double* set(matrix* M, int row, int col) {
  double* arr = M->data;
  int m = M->m;
  return arr + m*row + col;
}

int main(int argc,char** argv){
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
  guassian_eliminate(&A, &b);
  back_sub(&A, &b, &x);

  write_matrix(argv[3],x);
  free_matrix(A);
  free_matrix(b);
  free_matrix(x);
  
  return 0;
}

void guassian_eliminate(matrix* A, matrix* b) {
  int n = A->n;
  double* Ad = A->data;
  double* bd = b->data;
  
  int i, j, k;
  for(j = 0; j < n; j++) {
    for(i = j + 1; i < n; i++) {
      double mult = Ad[i*n + j]/Ad[j*n + j]; // A[i][j]/A[j][j];
      for(k = 0; k < n; k++) {
        Ad[i*n + k] = Ad[i*n+k] - mult*Ad[j*n+k]; // A[i][k] = A[i][k] - mult*A[j][k];
      }
      // Update b[i] also
      bd[i] = bd[i] - mult*bd[j]; // b[i] = b[i] - mult*b[j];
    }
  }
}

void back_sub(matrix* A, matrix* b, matrix* x) {
  int n = A->n;
  double* Ad = A->data;
  double* bd = b->data;
  double* xd = x->data;
  xd[n-1] = bd[n-1]/Ad[(n-1)*n + n-1]; // x[n-1] = b[n-1] / A[n-1][n-1];
  
  int i, j;
  for(i = n-2; i >= 0; i--) {
    double sum = 0;
    for(j = i+1; j < n; j++) {
      sum += Ad[i*n+j]*xd[j]; // A[i][j]*x[j];
    }
    xd[i] = (bd[i] - sum)/Ad[i*n + i]; // x[i] = (b[i] - sum)/A[i][i];
  }
}