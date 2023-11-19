#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <mpi.h>


int main(int argc, char* argv[]) {

  long i, count, n;
  double x,y,z,pi;
  double start, stop;
  int comm_sz, my_rank;
  
  MPI_Init(&argc, &argv); 

  if (argc < 2) {
    printf("Necessário informar um número N.\n");
    exit(-1);
  }
  n = strtoll(argv[1], NULL, 10);
  
   
   
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
  
  srand(my_rank);

  count = 0;

  start = MPI_Wtime();
  for (i=0; i < n; ++i) {

    x = (double)rand() / RAND_MAX;
    y = (double)rand() / RAND_MAX;

    z = x * x + y * y;

    if( z <= 1 ) count++;
  }
    
  long total_count;
  MPI_Reduce(&count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  stop = MPI_Wtime();
  
  if (my_rank == 0) {

    pi = ((double)total_count / (double)(n*comm_sz)) * 4;
    printf(" Nossa estimativa de pi = %.14f\n", pi);
    printf("                     pi = %.14f\n", 4.0*atan(1.0));
   
    fprintf(stdout, "Tempo gasto: %g s\n", stop-start);
  }
   
  MPI_Finalize(); 
  return(0);
}
