#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#define MASTER 0

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	srand(time(NULL));

	int* input;
	int rank, num_process;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);

	int* T = (int*)malloc(sizeof(int));
	int i;
	if(rank == MASTER) {
		printf("Please enter the number of trials to run: ");
		fflush(stdout);
		scanf("%d", T);


		for(i = 0; i < num_process; i++) {
			MPI_Send(T, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	} else {
		MPI_Recv(T, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);
	}

	int S = 0;
	float r;
	for(i = 0; i < *T; i++) {
		float x = (float)rand() / RAND_MAX;
		float y = (float)rand() / RAND_MAX;
		if(x*x + y*y < 1) {
			S++;
		}
	}
	float estimate = (float)4*S/(*T);

	if(rank != MASTER) {
		MPI_Send(&estimate, 1, MPI_FLOAT, MASTER, 0, MPI_COMM_WORLD);
	} else {
		float total = estimate;
		for(i = 0; i < num_process; i++) {
			MPI_Recv(&estimate, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &status);
			total += estimate;
		}

		float result = total/num_process;
		printf("The average estimate for Pi is: %.5f\n", result);
		fflush(stdout);
	}

	free(T);


	MPI_Finalize();
}
