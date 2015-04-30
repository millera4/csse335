#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define MASTER 0

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int* input;
	int rank, num_process;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);

	input = (int*) malloc(1 * sizeof(int));

	if(rank == MASTER) { // Master reads initial input
		printf("Enter an integer:\n");
		fflush(stdout);
		scanf("%d", input);
	}

	if(rank != MASTER) { // Everyone else receives
		MPI_Recv(input, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
		printf("Rank: %d Sender: %d Data: %d\n", rank, status.MPI_SOURCE, *input);
		fflush(stdout);
	}

	if(rank != num_process - 1) { // If not at the end, keep going
		MPI_Send(input, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	}
	free(input);

	MPI_Finalize();
}