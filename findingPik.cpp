#define _CRT_SECURE_NO_WARNINGS
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	double time = 0;
	int world_size, world_rank, nrPokemons, nrPikachus = 0, pikachusFound = 0, Pikachu = 55;
	FILE* fPokemons = fopen("C:/Users/desktop/source/repos/findingPik/pokemons2.txt", "r");
	fscanf(fPokemons, "%d", &nrPokemons);

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int maxP = nrPokemons / world_size;//number of pokemons scattered
	int* happyPokemons = NULL;//all pokemons
	int* sadPokemons = (int*)malloc(maxP * sizeof(int));//sad because they aren't togheter
	int* hapciu = NULL;//initially used to find pikachus from a specific world, then gathers them around
	int* pikapika = (int*)malloc(world_size * sizeof(int));//number of pikachus from each world

	if (world_rank == 0) {
		happyPokemons = (int*)malloc(nrPokemons * sizeof(int));
		for (int i = 0; i < nrPokemons; i++) {
			fscanf(fPokemons, "%d", happyPokemons + i);
		}
		time = MPI_Wtime();
	}

	MPI_Scatter(happyPokemons, maxP, MPI_INT, sadPokemons, maxP, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < maxP; i++) {
		if (*(sadPokemons + i) == Pikachu) {
			pikachusFound++;
			hapciu = (int*)realloc(hapciu, pikachusFound * sizeof(int));
			*(hapciu + pikachusFound - 1) = world_rank * maxP + i;
		}
	}

	if (world_rank != 0) {
		MPI_Send(&pikachusFound, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	else {
		*pikapika = nrPikachus = pikachusFound;
		for (int i = 1; i < world_size; i++) {
			MPI_Recv(pikapika + i, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			nrPikachus += *(pikapika + i);
		}
	}

	if (world_rank != 0) {
		MPI_Send(hapciu, pikachusFound, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	else {
		hapciu = (int*)realloc(hapciu, nrPikachus * sizeof(int));
		for (int q = pikachusFound, i = 1; i < world_size; q += *(pikapika + i), i++) {
			MPI_Recv(hapciu + q, *(pikapika + i), MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		for (int i = maxP * world_size; i < nrPokemons; i++) {
			if (*(happyPokemons + i) == Pikachu) {
				nrPikachus++;
				hapciu = (int*)realloc(hapciu, nrPikachus * sizeof(int));
				*(hapciu + nrPikachus - 1) = i;
			}
		}
		printf("It took %f s to find all the Pikachus out there. \n", MPI_Wtime() - time);
		printf("And there were %d Pikachus, at positions: \n", nrPikachus);
		for (int i = 0; i < nrPikachus; i++) {
			printf("%d ", *(hapciu + i));
		}
	}
	MPI_Finalize();
}