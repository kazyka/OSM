/*
 * Testing Spawn, Join and Exit
 */

#include "lib.h"

int main(void)
{
  int pid = syscall_spawn("[disk]hw.mips32", NULL);
  int retval = syscall_join(pid);
  printf("Return værdi: %d\n", retval);
  return 0;
}
