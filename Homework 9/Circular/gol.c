#include "gol_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void transition(gol_board* g, int t, int* displs, int* num_to_do, int rank){
  int i; int j; int row;

  enum STATUS mystatus;
  enum STATUS newstatus;
  int oc_nbrs;
  int todo = num_to_do[rank];
  for (row=0; row < todo; row++){
    i = displs[rank] + row;
    for (j=0;j<g->n;j++){
      mystatus=get_status(g,i,j,t);
      oc_nbrs=num_occupied_nbrs(g,i,j,t);
      if (mystatus==OCCUPIED){
	       if (oc_nbrs==2 || oc_nbrs==3) newstatus=OCCUPIED;
	       if (oc_nbrs>=4) newstatus=EMPTY;
	       if (oc_nbrs<2) newstatus=EMPTY;
      }
      if (mystatus==EMPTY){
	       if (oc_nbrs==3) newstatus=OCCUPIED;
	       else newstatus=EMPTY;
      }
      set_status(g,i,j,t+1,newstatus);
    }
  }
}

void assign_rows(int total, int numproc, int rank, int* num_all_do, int* displs) {
  int todo = total / numproc;
  if (total % numproc != 0)
    todo++;
  
  int sum = 0, i;
  for(i = 0; i < numproc - 1; i++) {
    num_all_do[i] = todo;
    displs[i] = sum;
    sum += todo;
  }
  num_all_do[numproc - 1] = total - (numproc - 1)*todo;
  displs[numproc - 1] = sum;
}

// Send my top row to the processor above, my bottom row to the processor below
// plus annoying setup to make 
void exchange_rows(gol_board* g, int t, int* displs, int* num_to_do, int rank, int numproc) {
  int above = rank - 1;
  if (above == -1) above = numproc - 1;
  int below = rank + 1;
  if (below == numproc) below = 0;
  
  int even = rank % 2 == 0;
  int first = even ? below : above;
  int second = even ? above : below;
  
  MPI_Status s;
  enum STATUS* base = g->data + t*g->m*g->n;

  enum STATUS* SEND1 = base + displs[rank]*g->n + (even ? g->n*(num_to_do[rank] - 1) : 0);
  enum STATUS* SEND2 = base + displs[rank]*g->n + (even ? 0 : g->n*(num_to_do[rank] - 1));
  enum STATUS* RECV1 = base + displs[first]*g->n + (even ? 0 : g->n*(num_to_do[first] - 1));
  enum STATUS* RECV2 = base + displs[second]*g->n + (even ? g->n*(num_to_do[second] - 1) : 0);
  
  MPI_Sendrecv(SEND1, g->n, MPI_INT, first, 0, RECV1, g->n, MPI_INT, first, 0, MPI_COMM_WORLD, &s);
  MPI_Sendrecv(SEND2, g->n, MPI_INT, second, 0, RECV2, g->n, MPI_INT, second, 0, MPI_COMM_WORLD, &s);
}

int main(int argc,char** argv) {
  MPI_Init(&argc, &argv);
  int rank, numproc;
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  gol_board g;
  options o; options defaults;
  strcpy(defaults.inputfile,"pulsar.txt");
  defaults.resolution[0]=512; defaults.resolution[1]=512;
  defaults.generations=100;
  o=parse_commandline(argc,argv,defaults);
  if(rank == 0)
    print_options(o);
  g.generations=o.generations;
  initialize_board(o.inputfile,&g);
  
  if(rank == 0)
    printf("Loaded a %dx%d board.\n",g.m,g.n);
  
  int* num_all_do = malloc(numproc*sizeof(int));
  int* displs = malloc(numproc*sizeof(int));
  
  assign_rows(g.m, numproc, rank, num_all_do, displs);
  
  int i;
  for (i=0;i<g.generations-1;i++){
    exchange_rows(&g, i, displs, num_all_do, rank, numproc);
    transition(&g, i, displs, num_all_do, rank);
  }
  
  // Collecting all the timesteps in the master's board.  whoops
  for(i = 0; i < numproc; i++) {
    num_all_do[i] *= g.n;
    displs[i] *= g.n;
  }
  for(i = 1; i < g.generations; i++) {
    int time_offset = i*g.m*g.n;
    int proc_offset = time_offset + displs[rank];
    MPI_Gatherv(g.data + proc_offset, num_all_do[rank], MPI_INT, g.data + time_offset, num_all_do, displs, MPI_INT, 0, MPI_COMM_WORLD);
  }
  
  //print_gol_board(&g);
  if(rank == 0)
    gol_board_to_gif(&g,o);
  
  MPI_Finalize();
  return (0);
}
