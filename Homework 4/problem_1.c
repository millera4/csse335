#include<stdlib.h>
#include<stdio.h>
#include<mpi.h>

#define MASTER 0

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int* input;
	int rank;
	int num_process;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);

	input = (int*) malloc(1 * sizeof(int));
	*input = 0;

	while(*input != -1) { 
		if(rank == MASTER) {
			printf("Enter an integer:\n ");
			fflush(stdout);
			scanf("%d", input);

			int i;
			for(i = 1; i < num_process; i++) {
				MPI_Send(input, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
		} else {
			MPI_Status status;
			MPI_Recv(input, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);
			if(*input == -1)
				break;

			printf("Rank: %d Received: %d\n", rank, *input);
			fflush(stdout);
		}
	}
	free(input);

	MPI_Finalize();
}
