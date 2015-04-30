#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include "nbodyutils.h"

void fill_results(vector* results, vector* data, int N, int offset) {
  // an array copy, used to put our current iteration into 
  // the result array
  int i;
  for(i = 0; i < N; i++) {
    results[offset + i] = data[offset + i];
  }
}

void evolve(sim_opts* s, nbody_dataset* d){
  // to send a vector -> {double, double} try and send MPI_LONG_LONG
  // it is the same type of data

  //x[i], v[i] are position and velocity VECTORS (literally vector structs)
  //m[i] is a mass array

  //x[i]' = v[i]
  //v[i]' = G*sum( m[j]*(x[j] - x[i])/ (norm(x[j]-x[i]))^3 )

  //Euler's method

  //x[i](t + h) = x[i](t) + hv[i](t)
  //v[i](t + h) = v[i](t) + h*G*sum( m[j] * (x[j](t) - x[i](t)) / norm(x[j]-x[i])^3 )
  int i, j;
  d->times[0] = 0; 

  // initialize t0 in results array
  fill_results(d->X, d->X0, d->N, 0);
  for(step = 1; step <= numsteps; step++) {


    d->times[step] = d->times[step - 1] + s.stepsize;
    fill_results(d->X, d->X0, d->N, d->N*step);
  }
}



void main(int argc,char** argv){

  nbody_dataset d;
  sim_opts s;


  read_sim_opts(argc,argv,&s);
  print_options(s);
  if (argc<3){

    print_usage();
    exit(1);
  }
  
  load_data(argv[1],&d);

  d.X=malloc(d.N*(s.numsteps+1)*sizeof(vector));
  d.times=malloc((s.numsteps+1)*sizeof(double));
  d.numsteps=s.numsteps;

  fprintf(stdout,"Read %d records.\n",d.N);
  fprintf(stdout,"Starting simulation.\n");

  evolve(&s,&d);
  
  fprintf(stdout,"Finished simulation\n");
  fprintf(stdout,"Writing data to %s\n",argv[2]);
  write_data(argv[2],&d);
  fprintf(stdout,"Done.\n");

}
