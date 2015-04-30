// Last updated: 4/22 for julia_parallel.c (Homework 6), using code from prime generation.

// A general outline of code that a master process will use (if it does no work). Minor changes
// should be all that is necessary to solve most kinds of problems in parallel (in terms of dynamic
// work allocation)

// Helpers the master will need: telling workers to quit, sending them work, and receiving results
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
  printf("Master got results for %d--%d\n",min,max);
  *c=max-min+1;
  *p=src;
}

// The master "main"
void be_a_master(options* o){
  int i;
  int* results=malloc((o->maxn+1)*sizeof(int));
  if (results==NULL){
    fprintf(stderr,"Error allocating memory for results on master.\n");
    MPI_Abort(MPI_COMM_WORLD,1);
  }

  double start=MPI_Wtime();
  
  results[0]=-1;
  results[1]=-1;

  int max_int_assigned=1;
  int num_ints_finished=2;

  int num_to_assign;
  for (i=1;i<=o->numworkers;i++){
    num_to_assign=min(o->taskchunksize,o->maxn-max_int_assigned);
    if (num_to_assign>0){
      send_work(i,max_int_assigned+1,max_int_assigned+num_to_assign);
      max_int_assigned+=num_to_assign;
    }
    else{
      send_quit(i);
    }
  }


  int p; int count;
  while (num_ints_finished<o->maxn+1){
    recv_results(results,&count,&p);
    num_ints_finished+=count;

    num_to_assign=min(o->taskchunksize,o->maxn-max_int_assigned);
    if (num_to_assign>0){
      send_work(p,max_int_assigned+1,max_int_assigned+num_to_assign);
      max_int_assigned+=num_to_assign;
    }
    else{
      send_quit(p);
    }
    
  }
  
  
  if (o->printresults){
    print_int_arr(results,o->maxn+1);
  }
  free(results);

  double end=MPI_Wtime();
  printf("This took a TOTAL of %lf seconds to complete.\n",end-start);
}