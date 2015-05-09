#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include "nbodyutils.h"


vector plus( vector v1, vector v2){
  vector s;
  s.x=v1.x+v2.x;
  s.y=v1.y+v2.y;
  return s;
}

vector minus(vector v1, vector v2){
  vector s;
  s.x=v1.x-v2.x;
  s.y=v1.y-v2.y;
  return s;
}

vector mult(vector v1, double alpha){
  vector s;
  s.x=alpha*v1.x;
  s.y=alpha*v1.y;
  return s;
}

void evolve(sim_opts* s, nbody_dataset* d){
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
     
      vector xi = d->X[offset + i-N];
      d->X[offset + i] = plus(xi, mult(d->V0[i], h));

      vector s; // V(t + h) result
      s.x = 0;
      s.y = 0;
      // Inner sum
      for(j = 0; j < N; j++) {
        if(j == i)
          continue;
        double distance = dist(d->X[offset + j - N], xi);
        double scalar = G*d->M[j]/(distance*distance*distance); // extra work
        vector diff = minus(d->X[offset + j - N], xi);
        s = plus(s, mult(diff, scalar));
      }
      d->V0[i] = plus(d->V0[i], mult(s,h));
           printf("%d.%d: X<%lf, %lf> V<%lf,%lf>\n", step, i, d->X[offset+i].x,d->X[offset+i].y,d->V0[i].x,d->V0[i].y);
    }

    // Update time
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
