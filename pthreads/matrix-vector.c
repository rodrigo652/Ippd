#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define SIZE 10000

long int M[SIZE][SIZE];
long int v1[SIZE], v2[SIZE];




int main()
{
  struct timeval start, stop;

  int i,j;
  for (i=0; i<SIZE; i++) {
    v1[i] = i;
    for (j=0; j<SIZE; j++)
      M[i][j] = i+j;
  }


  gettimeofday(&start, NULL);

  for (i=0; i<SIZE; i++) {
    v2[i] = 0;
    for(j=0; j<SIZE; j++) {
      v2[i] += M[i][j]*v1[j];
    }
  }
  
  gettimeofday(&stop, NULL);


  double temp = \
    (((double)(stop.tv_sec)*1000.0 + (double)(stop.tv_usec/1000.0)) - \
    ((double)(start.tv_sec)*1000.0 + (double)(start.tv_usec/1000.0)));

  fprintf(stdout, "Time elapsed = %g ms\n", temp);

  for (i=0; i<SIZE; i++)
    fprintf(stdout, "v[%d]=%ld\n", i, v2[i]);

	return 0;
}

