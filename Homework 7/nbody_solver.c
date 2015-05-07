#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<mpi.h>

#include "nbodyutils.h"


vector plus( vector v1, vector v2) {
  vector s;
  s.x=v1.x+v2.x;
  s.y=v1.y+v2.y;
  return s;
}

vector minus(vector v1, vector v2) {
  vector s;
  s.x=v1.x-v2.x;
  s.y=v1.y-v2.y;
  return s;
}

vector mult(vector v1, double alpha) {
  vector s;
  s.x=alpha*v1.x;
  s.y=alpha*v1.y;
  return s;
}

void evolve(sim_opts* s, nbody_dataset* d, int rank, int num_proc, MPI_Datatype MPI_VECTOR) {
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
  
  int num_to_do = N / num_proc;
  int extra = N % num_proc; // leftover data, X[0]..X[extra-1]
 
  
  // Results stored in X as follows: 
  //   X[0]..X[N-1] are X_i(0)
  //   X[N]..X[2N-1] are X_i(0 + h)
  //   ...
  //   X[N*step]...X[N*step + (N - 1)] are X_i(t + h)
  //
  //   V0[i] always has the current velocity. 
  for(step = 1; step <= numsteps; step++) {
    int time_offset = N*step;
    if (rank == 0) { // Master computes the extras
      for(i = 0; i < extra; i++) {
        vector xi = d->X[time_offset + i-N];
        d->X0[i] = plus(xi, mult(d->V0[i], h));
        
        vector sum_forces; // V(t + h) result
        sum_forces.x = 0;
        sum_forces.y = 0;
        // Inner sum
        for(j = 0; j < N; j++) {
          if(j == i)
            continue;
          double distance = dist(d->X[time_offset + j - N], xi);
          double scalar = d->M[j]/(distance*distance*distance); // extra work
          vector diff = minus(d->X[time_offset + j - N], xi);
          sum_forces = plus(sum_forces, mult(diff, scalar));
        }
        d->V0[i] = plus(d->V0[i], mult(sum_forces, h*G));
      }
    }
    
    // Everybody computes their own work
    int proc_offset = extra + rank*num_to_do;
    
    // x[i](t + h) = x[i](t) + h*v[i](t)
    // v[i](t+h) = v[i](t) + h*G*sum( m[j] * (x[j](t) - x[i](t)) / norm(x[j]-x[i])^3 )
    for(i = 0; i < num_to_do; i++) {
      vector xi = d->X[time_offset + proc_offset + i - N];
      d->X0[proc_offset + i] = plus(xi, mult(d->V0[proc_offset + i], h));
      
      vector s; // V(t + h) result
      s.x = 0;
      s.y = 0;
      // Inner sum
      for(j = 0; j < N; j++) {
        if(j == i)
          continue;
        double distance = dist(d->X[time_offset + j - N], xi);
        double scalar = d->M[j]/(distance*distance*distance); // extra work
        vector diff = minus(d->X[time_offset + j - N], xi);
        s = plus(s, mult(diff, scalar));
      }
      d->V0[proc_offset + i] = plus(d->V0[proc_offset + i], mult(s, h*G));
    }
    
    // Have master broadcast extra data to all processes
    if (rank == 0) {
      for(i = 0; i < extra; i++) {
        d->X[time_offset + i] = d->X0[i];
      }
    }
    MPI_Bcast((d->X + time_offset), extra, MPI_VECTOR, 0, MPI_COMM_WORLD);
    
    // Do an Allgather to send each node's info to the rest
    vector* sendV = malloc(N*sizeof(vector));
    for(i = 0; i < N; i++) {
      sendV[i] = d->V0[i];
    }
    
    MPI_Allgather(d->X0 + proc_offset, num_to_do, MPI_VECTOR, d->X, num_to_do, MPI_VECTOR, MPI_COMM_WORLD);
    MPI_Allgather(sendV + proc_offset, num_to_do, MPI_VECTOR, d->V0, num_to_do, MPI_VECTOR, MPI_COMM_WORLD);
    
    // Update time
    d->times[step] = d->times[step - 1] + s->stepsize;
  }
}



int main(int argc,char** argv) {
  int rank;
  int numproc;
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numproc);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  
  // Defining an MPI_VECTOR type
  MPI_Datatype MPI_VECTOR;
  int count = 1;
  int blocklens[1];
  blocklens[0] = 2;
  MPI_Aint offsets[1];
  offsets[0] = 0;
  MPI_Datatype oldtypes[1];
  oldtypes[0] = MPI_DOUBLE; 
  
  MPI_Type_struct(count, blocklens, offsets, oldtypes, &MPI_VECTOR);

  nbody_dataset d;
  sim_opts s;


  read_sim_opts(argc,argv,&s);
  MPI_Bcast(&d.N, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(rank == 0) {
    print_options(s);
  
    if (argc<3) {
      print_usage();
      MPI_Abort(MPI_COMM_WORLD,1);
    }
    
    load_data(argv[1],&d);
  } else {
    d.X0 = malloc(sizeof(vector)*d.N);
    d.V0 = malloc(sizeof(vector)*d.N);
    d.M = malloc(sizeof(double)*d.N);
    d.G = 6.674e-11;
  }
  // Sending out init data from master to workers
  MPI_Bcast(d.X0, d.N, MPI_VECTOR, 0, MPI_COMM_WORLD);
  MPI_Bcast(d.V0, d.N, MPI_VECTOR, 0, MPI_COMM_WORLD);
  MPI_Bcast(d.M, d.N, MPI_VECTOR, 0, MPI_COMM_WORLD);
  MPI_Bcast(&s.numsteps, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&s.stepsize, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  

  d.X=malloc(d.N*(s.numsteps+1)*sizeof(vector));
  d.times=malloc((s.numsteps+1)*sizeof(double));
  d.numsteps=s.numsteps;

  if (rank == 0) {
    fprintf(stdout,"Read %d records.\n",d.N);
    fprintf(stdout,"Starting simulation.\n");
  }
  
  evolve(&s,&d, rank, numproc, MPI_VECTOR);
  
  if (rank == 0) {
    fprintf(stdout,"Finished simulation\n");
    fprintf(stdout,"Writing data to %s\n",argv[2]);
    write_data(argv[2],&d);
    fprintf(stdout,"Done.\n");
  }
  
  MPI_Finalize();
  return 0;
}
