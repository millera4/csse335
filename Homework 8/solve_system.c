// Only set up to work for square matrices.  A lot of weird stuff can happen with 
// non-square matrices that isn't really relevant to being parallel.
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "solve_system_helpers.h"

void assign_rows(int* rows, int numrows, int numproc);
void guassian_eliminate(matrix* A, matrix* b, int rank, int* rows);
void back_sub(matrix* A, matrix* b, matrix* x);

int main(int argc,char** argv){
  MPI_Init(&argc, &argv);
  int rank, numproc;
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (argc<3){
    if(rank == 0) {
      printf("Usage: solve_system A b x\n");
      printf("Here A is the file storing the matrix A\n");
      printf("b is the file storing the vector b");
      printf("and x is the output file");
    }
    MPI_Abort(MPI_COMM_WORLD,1);
    exit(1);
  }

  matrix A=read_matrix(argv[1]);
  matrix b=read_matrix(argv[2]);
  matrix x=new_matrix(b.m,1);
  
  int* rows = malloc(A.m*sizeof(int));

  // do some stuff.
  // do_some_stuff(&A, &b, &x);
  assign_rows(rows, A.m, numproc);
  guassian_eliminate(&A, &b, rank, rows);
  
  if(rank == 0) {
    back_sub(&A, &b, &x);
    write_matrix(argv[3],x);
  }
  free_matrix(A);
  free_matrix(b);
  free_matrix(x);
  
  MPI_Finalize();
  return 0;
}

void assign_rows(int* rows, int numrows, int numproc) {
  int num_to_do = numrows / numproc;
  if (numrows % numproc != 0) {
    num_to_do++;
  }
  int i;
  int count = 0;
  int curr = 0;
  for(i = 0; i < numrows; i++) {
    rows[i] = curr;
    count++;
    if (count == num_to_do && curr != numproc - 1) {
      count = 0;
      curr++;
    }
  }
}

void guassian_eliminate(matrix* A, matrix* b, int rank, int* rows) {
  int n = A->n;
  double* Ad = A->data;
  double* bd = b->data;
  
  int i, j, k;
  for(i = 0; i < n; i++) {
    // Processor i broadcasts row i
    // do 2 broadcasts instead of appending b to A. efficiency weeps
    MPI_Bcast(Ad + i*n, n, MPI_DOUBLE, rows[i], MPI_COMM_WORLD);
    MPI_Bcast(bd + i, 1, MPI_DOUBLE, rows[i], MPI_COMM_WORLD);
    for(j = i + 1; j < n; j++) {
      // zero out my row
      if(rows[j] == rank) {
        double mult = Ad[j*n + i]/Ad[i*n + i]; // A[i][j]/A[j][j];
        for(k = 0; k < n; k++) {
          Ad[j*n + k] = Ad[j*n+k] - mult*Ad[i*n+k]; // A[i][k] = A[i][k] - mult*A[j][k];
        }
        // Update b[i] also
        bd[j] = bd[j] - mult*bd[i]; // b[i] = b[i] - mult*b[j];
      }
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