#include <stdio.h>  // printf, scanf, stdin
#include <ctype.h>  // isspace

// TODO: Maybe include queue.h?


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
      if (0) { // TODO: Try to pop an element off of the queue.
        printf("!! Queue underflow.\n");
      } else {
        printf("=> %d\n", pri);
      }
      break;

    default:
      ungetc(op, stdin);
      if (scanf("%d", &pri) == 1) {
        if (0) { // TODO: Try to insert the read priority into the queue.
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
  // TODO: Pop everything off of the queue.
}

int
main() {
  // TODO: Initialize the queue.

  loop();

  shutdown();

  // TODO: Destroy the queue.

  return 0;
}
