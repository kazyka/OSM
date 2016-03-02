#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "queue.h"

#define NUM_THREADS 4

static struct queue queue;

void *mythreadpush(void *arg) {

  printf("%s starting \n", (char *) arg);
  queue_push(&queue, 2);
  queue_push(&queue, 2);
  queue_push(&queue, 2);
  printf("%s ending \n", (char *) arg);
  return NULL;
}

void *mythreadpop(void *arg) {
  int pri = 0;

  printf("%s starting \n", (char *) arg);
  queue_pop(&queue, &pri);
  queue_pop(&queue, &pri);
  queue_pop(&queue, &pri);
  printf("%s ending \n", (char *) arg);
  return NULL;
}

int main() {
  queue_init(&queue);
  pthread_t threads[NUM_THREADS];
  int rc;
  printf("main: begin\n");
  rc = pthread_create(&threads[0], NULL, mythreadpush, "thread 0"); assert(rc == 0);
  rc = pthread_create(&threads[1], NULL, mythreadpop, "thread 1"); assert(rc == 0);
  rc = pthread_create(&threads[2], NULL, mythreadpush, "thread 2"); assert(rc == 0);
  rc = pthread_create(&threads[3], NULL, mythreadpop, "thread 3"); assert(rc == 0);
  // join waits for the threads to finish
  for (int i = 0; i < NUM_THREADS; i++) {
    rc = pthread_join(threads[i], NULL); assert(rc == 0);
  }
  printf("main: end\n");
  return 0;
}
