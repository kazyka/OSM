/*
 * Test userland semaphores.
 */

#include "lib.h"

int main(void) {
  syscall_usr_sem_open("test_sem", 1);
  return 0;
}
