#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

#include "image_utils.h"
#include "complex.h"
#include "julia_parallel_helpers.h"

#define MAX(a,b) (a)>(b)? (a): (b)
#define MIN(a,b) (a)<(b)? (a): (b)

// Computes the escape time for a complex z_n for a given number of iterations
// and a maximum norm
int escapeTime(complex zn, long int maxiter, double maxnorm, complex c) {
  int result, j;
  int flag = 1;
  double maxnorm_squared = maxnorm*maxnorm; // Should really calculate this before escapeTime
  for(j = 0; j < maxiter; j++) {
    zn = addc(multc(zn, zn), c);
    if(norm_squared(zn) > maxnorm_squared) {
      result = j;
      flag = 0;
      break;
    }
  }
  if(flag)
    result = maxiter;
  return result;
}

// Worker helper functions: receive work and send results back
int recv_work(int* min, int* max){
  MPI_Status status;
  int buffer[2];
  MPI_Recv(buffer,2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

  if (status.MPI_TAG==3){
    *min=buffer[0];
    *max=buffer[1];
    return 0;
  }
  else{
    return 1;
  } 
}

void send_results(int* results, int min, int max){
  int buffer[2]={min,max};
  MPI_Send(buffer,2,MPI_INT,0,1,MPI_COMM_WORLD);
  MPI_Send(results,max-min+1,MPI_INT,0,2,MPI_COMM_WORLD);
}

// Master helper functions: tell them to quit, send work, and receive results
void send_quit(int p) {
  MPI_Send(NULL, 0, MPI_INT, p, 4, MPI_COMM_WORLD);
}

void send_work(int p,int min, int max){
  int buffer[2]={min,max};
  MPI_Send(buffer,2,MPI_INT,p,3,MPI_COMM_WORLD);
}

void recv_results(int* results, int* c, int* p){
  int buffer[2];
  MPI_Status status;

  MPI_Recv(buffer,2,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);
  int min=buffer[0];
  int max=buffer[1];
  int src=status.MPI_SOURCE;
  MPI_Recv(results+min,max-min+1,MPI_INT,src,2,MPI_COMM_WORLD,&status);
  *c=max-min+1;
  *p=src;
}

// The worker algorithm
void be_a_worker(double rmin, double deltar, double imin, double deltai, int numsamples, long int maxiter, double maxnorm, complex c){
  double start=MPI_Wtime();
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int my_min;
  int my_max;

  int quit;
  int *my_results;
  quit=recv_work(&my_min,&my_max);
  int num_I_processed=0;
  while (!quit){
    my_results=malloc(sizeof(int)*(my_max-my_min+1));
    int i;
    complex z;
    for (i=my_min;i<=my_max;i++){

      // Converting a 1D array to 2D
      // Row := floor(value / rowlen); floor is automatic
      // Col := value % rowlen
      int row_offset = i / numsamples;
      int col_offset = i % numsamples;

      // Reals are the columns, Imagines are the rows
      z.real = rmin + col_offset*deltar;
      z.imag = imin + row_offset*deltai;

      // Results stores all the times
      my_results[i-my_min] = escapeTime(z, maxiter, maxnorm, c);
    }
    send_results(my_results,my_min,my_max);
    num_I_processed+=my_max-my_min+1;
    free(my_results);
    quit=recv_work(&my_min,&my_max);
  }

  double end=MPI_Wtime();
  printf("Rank %d spend %lf time processing %d integers.\n",rank,end-start,num_I_processed);
}

// The master algorithm
void be_a_master(int numsamples, long int maxiter, double maxnorm, char* outputfile, complex c, int forcebyteswap, int staticallocation, int linechunksize, int numworkers) {
  int i;
  int* results=malloc(numsamples*numsamples*sizeof(int));
  if (results==NULL){
    fprintf(stderr,"Error allocating memory for results on master.\n");
    MPI_Abort(MPI_COMM_WORLD,1);
  }

  double start=MPI_Wtime();

  int chunks_assigned=-1;
  int chunks_finished=0;
  int maxn = numsamples*numsamples;

  int num_to_assign;
  for (i=1;i<=numworkers;i++){
    num_to_assign=MIN(linechunksize,maxn-chunks_assigned);
    if (num_to_assign>0){
      send_work(i,chunks_assigned+1,chunks_assigned+num_to_assign);
      chunks_assigned+=num_to_assign;
    }
    else{
      send_quit(i);
    }
  }


  int p; int count;
  while (chunks_finished<maxn+1){
    recv_results(results,&count,&p);
    chunks_finished+=count;
    num_to_assign=MIN(linechunksize,maxn-chunks_assigned);
    if (num_to_assign>0){
      send_work(p,chunks_assigned+1,chunks_assigned+num_to_assign);
      chunks_assigned+=num_to_assign;
    }
    else{
      send_quit(p);
    }
    
  }

  // Create the image, first we need a double array for fun
  double* g = malloc(numsamples*numsamples*sizeof(double));
  for(i = 0; i < numsamples*numsamples; i++) {
    g[i] = (double)results[i];
  }
  write_greyscale_bmp(outputfile, g, numsamples, numsamples, 0, maxiter, forcebyteswap);
  free(outputfile);
  free(results);
  free(g);

  double end=MPI_Wtime();
  printf("This took a TOTAL of %lf seconds to complete.\n",end-start);
}


int main(int argc, char** argv){

  char* outputfile;
  double rmin;
  double rmax;
  double imin;
  double imax;
  long int maxiter;
  int numsamples;
  double maxnorm;

  complex c;

  int forcebyteswap;

  int rank;
  int numproc;
  int staticallocation;
  int linechunksize;
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numproc);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  
  set_opts(argc,argv,&rmin, &rmax, &imin, &imax, &numsamples, &maxiter, &maxnorm, &outputfile,&c,&forcebyteswap,&staticallocation,&linechunksize);

  double deltar=(rmax-rmin)/numsamples;
  double deltai=(imax-imin)/numsamples;

  if (staticallocation){
    linechunksize=MIN(numsamples/(numproc-1)+1,numsamples);
  }

  
  
  if (rank==0) {
    double start=MPI_Wtime();
    printf("Processors: %d\nAllocation Method: %s\nline-chunk-size: %d\ndeltai:%f\ndeltar:%f\nmaxiter:%ld\nmaxnorm:%lf\noutput file: %s\nc:%f+%fi\n\n\n",numproc,staticallocation==0?"Dynamic":"Static",linechunksize,deltai,deltar,maxiter,maxnorm,outputfile,c.real,c.imag); 
    int numworkers=numproc-1;
    if (numworkers<1){
      fprintf(stderr,"Wow I was never meant to be serial.\n");
      MPI_Abort(MPI_COMM_WORLD,1);
    }

    be_a_master(numsamples, maxiter, maxnorm, outputfile, c, forcebyteswap, staticallocation, linechunksize, numworkers);
  }
  else {
    if(numproc-1 < 1) {
      fprintf(stderr,"Wow I was never meant to be serial.\n");
      MPI_Abort(MPI_COMM_WORLD,1);
    }
    be_a_worker(rmin, deltar, imin, deltai, numsamples, maxiter, maxnorm, c);
  } 
  
  MPI_Finalize();
  return 0;
}
