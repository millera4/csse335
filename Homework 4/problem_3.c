#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#define MASTER 0

// Helper declarations
void random_processes(int* p1, int* p2, int total);
double comm_with_master(int rank, char* message, int size, int samples);
double comm(int* info, char* message, int size, int samples);

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	// Init and process input
	srand(time(NULL));
	int rank, num_process;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_process);

	char name[1000];
	gethostname(name, 1000);

	int P = atoi(argv[1]);
	int B = atoi(argv[2]);
	int E = atoi(argv[3]);

	if (P % 2 != 0) {
		if(rank == MASTER) {
			printf("The number of processes (arg 1) must be even.\n");
			fflush(stdout);
		}
		MPI_Finalize();
		exit(0);
	}

	// Main loop
	int* STOP = (int*)malloc(sizeof(int));
	*STOP = -1; // Signal to stop
	char* message = (char*)malloc(B*sizeof(char));

	if(rank == MASTER) {
		printf("Data Size: %dB\n", B);
		printf("Number of Samples: %d\n", E);
		fflush(stdout);

		double* test = (double*) malloc(sizeof(double));
		while(P != 0) {
			int* p1 = (int*)malloc(sizeof(int));
			int* p2 = (int*)malloc(sizeof(int));
			random_processes(p1, p2, num_process);
			*(p1 + 1) = 1; // p1 will send first
			*(p2 + 1) = 0; // p2 will receive first

			
			if(*p1 == MASTER) {
				*(p1 + 1) = 0;
				MPI_Send(p1, 2, MPI_INT, *p2, 0, MPI_COMM_WORLD);
				*test = comm_with_master(*p2, message, B, E);
			} else if(*p2 == MASTER) {
				MPI_Send(p2, 2, MPI_INT, *p1, 0, MPI_COMM_WORLD);
				*test = comm_with_master(*p1, message, B, E);
			} else {
				MPI_Send(p2, 2, MPI_INT, *p1, 0, MPI_COMM_WORLD);
				MPI_Send(p1, 2, MPI_INT, *p2, 0, MPI_COMM_WORLD);

				// only get result from 1 source
				MPI_Recv(test, 1, MPI_DOUBLE, *p2, 0, MPI_COMM_WORLD, &status);
			}
			double speed = *test / 1000000; // test is Bytes/Sec. Want MB/s
			printf("Communication Speed between rank %d and %d was %.1f MB/s\n", *p1, *p2, speed);
			P -= 2;
		}
		printf("\nHere is the machine-process pairing\n\n");
		fflush(stdout);
		int i;
		for(i = 1; i < num_process; i++) {
			MPI_Send(STOP, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	} else {
		int* result = (int*) malloc(2*sizeof(int));
		double* test = (double*) malloc(sizeof(double));
		while (1) { // Don't judge
			MPI_Recv(result, 2, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);
			if(*result == *STOP)
				break;
			*test = comm(result, message, B, E);
			if(*(result + 1) == 1) {
				MPI_Send(test, 1, MPI_DOUBLE, MASTER, 0, MPI_COMM_WORLD);
			}
		}
		free(result);
		free(test);
	}
	free(stop);
	free(message);

	printf("Rank %d: %s\n", rank, name);
	fflush(stdout);

	MPI_Finalize();
}

void random_processes(int* p1, int* p2, int num_process) {
	*p1 = rand() % num_process; // Randomly choose a process;

	do {
		*p2 = rand() % num_process; // Randomly choose p2
	} while(*p1 == *p2); // Until we get a distinct pair
}

double comm_with_master(int rank, char* message, int size, int samples) {
	MPI_Status status;
	double avg_time = 0;
	double start_time, end_time;
	int i;
	for(i = 0; i < samples; i++) {
		start_time = MPI_Wtime();
		MPI_Send(message, size, MPI_CHAR, rank, 0, MPI_COMM_WORLD);
		MPI_Recv(message, size, MPI_CHAR, rank, 0, MPI_COMM_WORLD, &status);
		end_time = MPI_Wtime();

		// 2N/T
		avg_time += 2*size/(end_time - start_time);	
	}
	return avg_time / samples;
}

double comm(int* data, char* message, int size, int samples) {
	MPI_Status status;
	double avg_time = 0;
	double start_time, end_time;
	int i;
	for(i = 0; i < samples; i++) {
		if(*(data + 1) == 1) { // begins sending
			start_time = MPI_Wtime();
			MPI_Send(message, size, MPI_CHAR, *data, 0, MPI_COMM_WORLD);
			MPI_Recv(message, size, MPI_CHAR, *data, 0, MPI_COMM_WORLD, &status);
			end_time = MPI_Wtime();

			// 2N/T
			avg_time += 2*size/(end_time - start_time);	
		} else {
			MPI_Recv(message, size, MPI_CHAR, *data, 0, MPI_COMM_WORLD, &status);
			MPI_Send(message, size, MPI_CHAR, *data, 0, MPI_COMM_WORLD);
		}
	}

	return avg_time / samples;
}