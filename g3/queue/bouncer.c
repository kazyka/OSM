#include <stdio.h>  // printf, scanf, stdin
#include <ctype.h>  // isspace

#include "queue.h"

static struct queue queue;

static inline char
skip_spaces() {
  char c;

  do {
    c = getc(stdin);
  } while (isspace(c));

  return c;
}

static inline void
skip_until_space() {
  char c;

  do {
    c = getc(stdin);
  } while (!(isspace(c) || c == EOF));
}

void
loop() {
  char op = 0;
  int pri = 0;

  while(1) {

    op = skip_spaces();

    switch (op) {
    case EOF:
      return;
      break;

    case 'p':
      if (queue_pop(&queue, &pri) != 0) {
        printf("!! Queue underflow.\n");
      } else {
        printf("=> %d\n", pri);
      }
      break;

    default:
      ungetc(op, stdin);
      if (scanf("%d", &pri) == 1) {
        if (queue_push(&queue, pri) != 0) {
          printf("!! Queue overflow.\n");
        }
      } else {
        skip_until_space();
        printf("Invalid input!\n");
      }
    }
  }
}

void
shutdown() {
  int pri;

  while (queue_pop(&queue, &pri) == 0)
    ;
}

int
main() {
  queue_init(&queue);

  loop();

  shutdown();

  queue_destroy(&queue);

  return 0;
}
