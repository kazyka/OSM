/* Test userland processes. */

#include "lib.h"

int main() {
  int pid = syscall_spawn("[disk]hw.mips32", NULL);
  int retval = syscall_join(pid);
  printf("returværdi: %d\n", retval);
  
  syscall_exit(0);
  return 0;
}
