#ifndef SOLVE_SYSTEM_HELPERS
#define SOLVE_SYSTEM_HELPERS

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

typedef struct matrix{
  double* data;
  int m;
  int n;

}matrix;

matrix read_matrix(char* filename){

  FILE* fp=fopen(filename,"r");

  if (fp==NULL){
    fprintf(stderr,"Error opening %s for reading. \n",filename);
    exit(1);
  }
  matrix A;

  if (fscanf(fp,"%d,%d",&(A.m),&(A.n))!=2){
    fprintf(stderr,"Error reading line 1 of %s.\n",filename);
    exit(1);
  }

  int i;
  int j;
  A.data=malloc(sizeof(double)*A.m*A.n);
  for (i=0;i<A.m;i++){
    if (fscanf(fp,"%lf",A.data+A.n*i)!=1){
      fprintf(stderr,"Error reading %s.\n",filename);
      exit(1);
    }
    for (j=1;j<A.n;j++){
      if (fscanf(fp,",%lf",A.data+A.n*i+j)!=1){
	fprintf(stderr,"Error reading %s.\n",filename);
	exit(1);
      }
    }
  }

  fclose(fp);
  return A;


}


matrix new_matrix(int m, int n){

  matrix A;
  A.m=m;
  A.n=n;
  A.data=malloc(m*n*sizeof(double));

  return A;
}


void free_matrix(matrix A){
  free(A.data);
}

void fprintmatrix(FILE* fp,matrix A,char delim){

  int i;
  int j;
  for (i=0;i<A.m;i++){
    for (j=0;j<A.n;j++){
      fprintf(fp,"%lf",A.data[i*A.n+j]);
      if (j<A.n-1){
	fprintf(fp,"%c",delim);
      }
    }
    fprintf(fp,"\n");
  }
    
}

void write_matrix(char* filename, matrix A){

  FILE* fp=fopen(filename,"w");
  if (fp==NULL){
    fprintf(stderr,"Error opening %s for writing.\n",filename);
    exit(1);
  }

  fprintf(fp,"%d,%d\n",A.m,A.n);
  fprintmatrix(fp,A,',');
  
  fclose(fp); 


}

void printmatrix(matrix A){
  fprintmatrix(stdout,A,' ');
}


matrix random_matrix(int m,int n,int min, int max){

  matrix A=new_matrix(m,n);
  int i;
  int j;

  srand(time(NULL));
  for (i=0;i<m;i++){
    for (j=0;j<n;j++){
      A.data[i*n+j]=rand()%(max-min)+min;
    }
  }
  return A;
  
}

#endif


  
