#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "queue.h"

static struct queue queue;

void *mythread1(void *arg) {
  int pri = 0;

  printf("%s\n", (char *) arg);
  queue_pop(&queue, &pri);
  queue_pop(&queue, &pri);
  queue_pop(&queue, &pri);
  queue_push(&queue, 1);
  queue_pop(&queue, &pri);
  printf("%s ending\n", (char *) arg);
  return NULL;
}

void *mythread2(void *arg) {
  int pri = 0;

  printf("%s\n", (char *) arg);
  queue_pop(&queue, &pri);
  queue_push(&queue, 2);
  queue_push(&queue, 2);
  queue_push(&queue, 2);
  queue_pop(&queue, &pri);
  printf("%s ending\n", (char *) arg);
  return NULL;
}

int main() {
  queue_init(&queue);
  pthread_t p1, p2;
  int rc;
  printf("main: begin\n");
  rc = pthread_create(&p1, NULL, mythread1, "A"); assert(rc == 0);
  rc = pthread_create(&p2, NULL, mythread2, "B"); assert(rc == 0);
  // join waits for the threads to finish
  rc = pthread_join(p1, NULL); assert(rc == 0);
  rc = pthread_join(p2, NULL); assert(rc == 0);
  printf("main: end\n");
  return 0;
}
