#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <pthread.h>

#define SIZE 10000

long int M[SIZE][SIZE];
long int v1[SIZE], v2[SIZE];


int thread_count; 


void *mvthread (void *arg)
{
 
  int my_id = (int)arg;

  int block = SIZE/thread_count;
  int resto = SIZE%thread_count;

  int my_i = my_id*block;

  // versao paralela	
  int i,j;
  for (i=my_i; i<my_i+block; i++) {
    v2[i] = 0;
    for(j=0; j<SIZE; j++) {
      v2[i] += M[i][j]*v1[j];
    }
  }

  if ((my_id == thread_count-1) && (resto != 0)) {
    for (i=my_i+block; i<my_i+block+resto; i++) {
      v2[i] = 0;
      for(j=0; j<SIZE; j++) {
        v2[i] += M[i][j]*v1[j];
      }
    }
  }



  return NULL;
}




int main(int argc, char *argv[])
{
  struct timeval start, stop;
  pthread_t *thread_handles;

  thread_count = strtol(argv[1], NULL, 10);
  thread_handles = malloc(thread_count*sizeof(pthread_t));

  int i,j;
  for (i=0; i<SIZE; i++) {
    v1[i] = i;
    for (j=0; j<SIZE; j++)
      M[i][j] = i+j;
  }

  gettimeofday(&start, NULL);
  // FORK
  for (i=0; i<thread_count; i++) {
    if (pthread_create(&thread_handles[i], NULL, mvthread, (void *)i) != 0) {
      fprintf(stderr, "Nao consegui criar a thread\n");
      exit(-1);
    }
  }
  
  /* JOIN */
  for (i=0; i<thread_count; i++)
  {
    pthread_join(thread_handles[i], NULL);
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

