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

void evolve(sim_opts* s, nbody_dataset* d, int rank, int num_proc) {
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
  
  // Setting up send/recv counts for displs
  int* num_to_send = malloc(num_proc*sizeof(int));
  int* displ = malloc(num_proc*sizeof(int));
  
  int num_to_do = N / num_proc;
  if (N % num_proc != 0) {
    num_to_do++;
  }
  int sum = 0;
  for(i = 0; i < num_proc - 1; i++) {
    num_to_send[i] = num_to_do;
    displ[i] = sum;
    sum += num_to_do;
  }
  // the last processor does the rest
  num_to_send[num_proc - 1] = N - (num_proc - 1)*num_to_do;
  displ[num_proc - 1] = sum;
  
  // Defining an MPI_VECTOR type
  MPI_Datatype MPI_VECTOR;
  int typecount = 1;
  int blocklens[1] = { 2 };
  MPI_Aint offsets[1] = { 0 };
  MPI_Datatype oldtypes[1] = { MPI_DOUBLE };  
  MPI_Type_create_struct(typecount, blocklens, offsets, oldtypes, &MPI_VECTOR);
  MPI_Type_commit(&MPI_VECTOR);
  
  for(step = 1; step <= numsteps; step++) {
    int time_offset = N*step;
    int proc_offset = displ[rank];
    
    // x[i](t + h) = x[i](t) + h*v[i](t)
    // v[i](t+h) = v[i](t) + h*G*sum( m[j] * (x[j](t) - x[i](t)) / norm(x[j]-x[i])^3 )
    for(i = 0; i < num_to_send[rank]; i++) {
      vector xi = d->X[time_offset + proc_offset + i - N];
      d->X0[proc_offset + i] = plus(xi, mult(d->V0[proc_offset + i], h));
      
      vector s; // V(t + h) result
      s.x = 0;
      s.y = 0;
      // Inner sum
      for(j = 0; j < N; j++) {
        if(j == i + proc_offset)
          continue;
        double distance = dist(d->X[time_offset + j - N], xi);
        double scalar = d->M[j]/(distance*distance*distance); // extra work
        vector diff = minus(d->X[time_offset + j - N], xi);
        s = plus(s, mult(diff, scalar));
      }
      d->V0[proc_offset + i] = plus(d->V0[proc_offset + i], mult(s, h*G));
    }
    
    // Do an Allgather to send each node's info to the rest
    vector* sendV = malloc(N*sizeof(vector));
    for(i = 0; i < N; i++) {
      sendV[i] = d->V0[i];
    }
    MPI_Allgatherv(d->X0 + proc_offset, num_to_send[rank], MPI_VECTOR, d->X+time_offset, num_to_send, displ, MPI_VECTOR, MPI_COMM_WORLD);
    MPI_Allgatherv(sendV + proc_offset, num_to_send[rank], MPI_VECTOR, d->V0, num_to_send, displ, MPI_VECTOR, MPI_COMM_WORLD);
    
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

  nbody_dataset d;
  sim_opts s;


  read_sim_opts(argc,argv,&s);
  if(rank == 0) {
    print_options(s);
  
    if (argc<3) {
      print_usage();
      MPI_Abort(MPI_COMM_WORLD,1);
    }
  }
  load_data(argv[1],&d);

  d.X=malloc(d.N*(s.numsteps+1)*sizeof(vector));
  d.times=malloc((s.numsteps+1)*sizeof(double));
  d.numsteps=s.numsteps;

  if (rank == 0) {
    fprintf(stdout,"Read %d records.\n",d.N);
    fprintf(stdout,"Starting simulation.\n");
  }
  
  evolve(&s,&d, rank, numproc);
  
  if (rank == 0) {
    fprintf(stdout,"Finished simulation\n");
    fprintf(stdout,"Writing data to %s\n",argv[2]);
    write_data(argv[2],&d);
    fprintf(stdout,"Done.\n");
  }
  
  MPI_Finalize();
  return 0;
}
