#include <stdio.h>  // printf, scanf, stdin
#include <ctype.h>  // isspace
#include "queue.h"  // Queue struct

struct queue queue;
int pri_ptr;

static inline char skip_spaces() {
  char c;

  do {
    c = getc(stdin);
  } while (isspace(c));

  return c;
}

static inline void skip_until_space() {
  char c;

  do {
    c = getc(stdin);
  } while (!(isspace(c) || c == EOF));
}

void loop() {
  char op = 0;
  int pri = 0;

  while(1) {

    op = skip_spaces();

    switch (op) {
    case EOF:
      return;
      break;

    case 'p':
      // Pop an element off of the queue.
      if (queue_pop(&queue, &pri_ptr) != 0) {
        printf("!! Queue underflow.\n");
      } else {
        printf("=> %d\n", pri_ptr);
      }
      break;

    case 'e': // Manual exit command
       return;
       break;

    case 'q': // Got annoyed of accidentally typing q instead
              // of e so added support for both
       return;
       break;

    default:
      ungetc(op, stdin);
      if (scanf("%d", &pri) == 1) {
        // Push the object

        queue_push(&queue, pri);
        if (0) {
          // We can at maximum insert one element at a time.
          // With a dynamic array, queue overflow should not occur
          printf("!! Queue overflow.\n");
        }
      } else {
        skip_until_space();
        printf("Invalid input!\n");
      }
    }
  }
}

void shutdown() {
  // Pop everything off of the queue.
  int pri_ptr2;

  while (queue.array_size > 0){
     queue_pop(&queue, &pri_ptr2);
     queue.array_size = queue.array_size - 1;
  }
}

int main() {
  queue_init(&queue);

  loop();

  //print_stuff(&queue);
  
  shutdown();

  queue_destroy(&queue);


  return 0;
}
