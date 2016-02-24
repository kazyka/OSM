/*
 * syscall_read/syscall_write test program
 *
 * Uses library implementation of printf instead of using syscall_write
 * directly.
 */

#include "lib.h"
#include "../kudos/proc/syscall.h"

#define LENGTH (64)

int main(void) {
  char buffer[LENGTH];
  int retval;

  // Positive tests.

  printf("I am KUDOS userland, who are you?\n");
  retval = syscall_read(FILEHANDLE_STDIN, &buffer, LENGTH);
  if (retval < 0) {
    printf("syscall_read failed with %d :-(\n", retval);
    syscall_halt();
  }
  printf("Hello, %s.\n", buffer);

  printf("This is the end, my only friend, the end..\n");

  // No negative tests, too lazy.

  syscall_halt();
  return 0;
}
