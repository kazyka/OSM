/*
 * testing for read syscalls
 */

#include "lib.h"

int main(void) {
  heap_init();
  void *buffer = malloc(BUFSIZE);
  syscall_read(0, buffer, BUFSIZE);
  syscall_write(1, buffer, strlen(buffer));
  syscall_halt();
  return 0;
}
