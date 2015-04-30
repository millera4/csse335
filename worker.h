// Last updated: 4/22 for julia_parallel.c (Homework 6), using code from prime generation.

// A general outline of code that a worker process will use. Minor changes should be all 
// is necessary to solve most kinds of problems in parallel (in terms of dynamic work allocation)

// Worker helper functions: receive work and send results back
int recv_work(int* min, int* max){
  MPI_Status status;
  int buffer[2];
  MPI_Recv(buffer,2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

  if (status.MPI_TAG==3){
    *min=buffer[0];
    *max=buffer[1];
    printf("Got assignement %d--%d\n",*min,*max);

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

// The worker "main"
void be_a_worker(options* o){
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
    for (i=my_min;i<=my_max;i++){
      my_results[i-my_min]=is_prime(i);
    }
    send_results(my_results,my_min,my_max);
    num_I_processed+=my_max-my_min+1;
    free(my_results);
    quit=recv_work(&my_min,&my_max);
  }

  double end=MPI_Wtime();
  printf("Rank %d spend %lf time processing %d integers.\n",rank,end-start,num_I_processed);
}