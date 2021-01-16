#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char *argv[]) 
{
    int rank, size;
    int *array_in;
    int *array_out;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);	
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    printf("MPI rank: %d size: %d\n",rank, size);    
    
    if (rank==0)
      printf("Demo MPI_Alltoall: jeder rank verteilt dem Wert 11*rank an alle anderen Prozesse\n");
    
    array_in = malloc(size*sizeof(int));
    array_out = malloc(size*sizeof(int));
    /* ohne Fehlerbehandlung, da nur ein Testprogramm */
    
    
    // Variante 1:    
    // jeder Prozess schreibt Wert für prozess j an die Indexpos. j auf array_in.
    // Falls ein Prozess an alle anderen den gleichen Wert sendet, dann 
    // schreibt er diesen Wert auf alle Elemente des Feldes 
    
    for (int i=0; i<size;i++)
      array_in[i] = 11*rank;
          
    /* sowohl array_in, als auch array_out muss ein Feld mit size Einträgen sein */
    MPI_Alltoall(array_in, 1, MPI_INT, array_out, 1, MPI_INT, MPI_COMM_WORLD);
    
    if (rank==size/2)
    {
      printf("Rank %d received:\n", rank);
      for (int i=0;i<size;i++)
        printf(" %d ",array_out[i]);
      printf("\n");
    }
    

    // In-Place Datenaustausch, MPI_IN_PLACE anstelle array_in
    
    for (int i=0;i<size;i++)
       array_out[i] = 11*rank;    
    /* sowohl array_in, als auch array_out muss ein Feld mit size Einträgen sein */
    MPI_Alltoall(MPI_IN_PLACE, 0, MPI_INT, array_out, 1, MPI_INT, MPI_COMM_WORLD);
    
    if (rank==size/2)
    {
      printf("Rank %d received:\n", rank);
      for (int i=0;i<size;i++)
        printf(" %d ",array_out[i]);
      printf("\n");
    }
    
    
    free(array_in);
    free(array_out);
    
    MPI_Finalize();
    return 0;
}