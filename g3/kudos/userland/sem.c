/*
 * Test userland semaphores.
 */

#include "lib.h"
#include "lib/types.h"

int main (void) {
  usr_sem_t* test_sem = syscall_usr_sem_open("test_sem", 5);
  syscall_usr_sem_destroy(&test_sem);
  return 0;
}
