/* TUW, October 2011 */

#include <stdio.h>
#include <stdlib.h>

// pthreads header
#include <pthread.h>

// for checking
#include <assert.h>

#include <time.h>
// clocks
// CLOCK_PROCESS_CPUTIME_ID
// CLOCK_MONOTONIC
/*
  struct timespec tstart, tend;
  clock_gettime(CLOCK_MONOTONIC,&tstart);
*/

// dangerous example of global state; here read-only
int threads_glob;

// mutexes also should not be global
pthread_mutex_t printer = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  int parent, subrank;
} somearg_t;

void *subsome(void *argument)
{
  somearg_t *ranks = (somearg_t*)argument;

  printf("Subthread %d of %d now responding\n",ranks->subrank,ranks->parent);

  pthread_exit(NULL);
}

void *something(void *argument)
{
  int rank = *(int*)argument;
  // misuse of void pointer
  //int rank = (long)argument;
  int i;

  pthread_t subhandle[3];
  somearg_t arguments[3];

  pthread_mutex_lock(&printer); // not needed, for illustration purposes
  printf("Thread with assigned rank %d of %d responding\n",rank,threads_glob);
  pthread_mutex_unlock(&printer);

  // create and join three subthreads
  for (i=0; i<0; i++) {
    arguments[i].parent = rank;
    arguments[i].subrank = i;

    pthread_create(&subhandle[i],NULL,subsome,&arguments[i]);
    pthread_join(subhandle[i],NULL);
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  int threads, *rank;
  int i;
  int errcode;

  pthread_t *handle;

  threads = 1;
  for (i=1; i<argc&&argv[i][0]=='-'; i++) {
    if (argv[i][1]=='t') i++,sscanf(argv[i],"%d",&threads);
  }

  threads_glob = threads;

  handle = (pthread_t*)malloc(threads*sizeof(pthread_t));
  rank = (int*)malloc(threads*sizeof(int));

  // fork the threads
  for (i=0; i<threads; i++) {
    // correct, but misuse
    //errcode = pthread_create(&handle[i],NULL,something,(void*)(long)i);
    rank[i] = i;
    errcode = pthread_create(&handle[i],NULL,something,&rank[i]);
    // wrong
    //errcode = pthread_create(&handle[i],NULL,something,&i);
    assert(errcode==0);
  }
  
  // join the threads again
  for (i=0; i<threads; i++) {
    pthread_join(handle[i],NULL);
  }

  free(rank);
  free(handle);
  return 0;
}
