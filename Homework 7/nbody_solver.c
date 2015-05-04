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

// Computes k*v, where k is a scalar and v is a vector
vector v_mult_scalar(double k, vector v) {
  vector result;
  result.x = k*v.x;
  result.y = k*v.y;
  return result;
}

// Computes a + b
vector v_add(vector a, vector b) {
  vector result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

// Computes a - b
vector v_sub(vector a, vector b) {
  vector result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

void evolve(sim_opts* s, nbody_dataset* d){
  // to send a vector -> {double, double} try and send MPI_LONG_LONG
  // it is the same length of data

  //v[i](t + h) = v[i](t) + h*G*sum( m[j] * (x[j](t) - x[i](t)) / norm(x[j]-x[i])^3 )
  int i, j, step;
  d->times[0] = 0; 
  double h = s->stepsize;
  double G = d->G;
  int N = d->N;
  int numsteps = s->numsteps;

  // initialize t0 in results arrays
  for(i = 0; i < N; i++) {
    d->X[i] = d->X0[i];
  }
  
  // Results stored in X as follows: 
  //   X[0]..X[N-1] are X_i(0)
  //   X[N]..X[2N-1] are X_i(0 + h)
  //   ...
  //   X[N*step]...X[N*step + (N - 1)] are X_i(t + h)
  //
  //   V0[i] always has the current velocity. 
  for(step = 1; step <= numsteps; step++) {
    int offset = N*step;
    
    // x[i](t + h) = x[i](t) + h*v[i](t)
    // v[i](t+h) = v[i](t) + h*G*sum( m[j] * (x[j](t) - x[i](t)) / norm(x[j]-x[i])^3 )
    for(i = 0; i < d->N; i++) {
      // doing this in one line because I'm mean
      d->X[offset + i] = v_add(d->X[offset + i - N], v_mult_scalar(h,d->V0[i]));
      
      vector xi = d->X[offset + i-N];
      vector s = d->V0[i]; // V(t + h) result
      // Inner sum
      for(j = 0; j < N; j++) {
        double distance = dist(d->X[offset + j - N], xi);
        double scalar = h*G*d->M[j]/(distance*distance*distance); // extra work
        vector diff = v_sub(d->X[offset + j - N], xi);
        s = v_add(s, v_mult_scalar(scalar, diff));
      }
      d->V0[i] = s;
    }
    

    // Store this over the 
    d->times[step] = d->times[step - 1] + s->stepsize;
  }
}



int main(int argc,char** argv){

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
  return 0;
}
