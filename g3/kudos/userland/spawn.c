/*
 * Test that spawn works to the extent needed (we must be able to spawn new
 * programs, but we don't have to keep track of them..
 */

#include "lib.h"

int main(void) {
  puts("Spawning two programs...\n");
  syscall_spawn("[disk]hello.mips32", NULL);
  syscall_spawn("[disk]hw.mips32", NULL);

  while (1); /* Loop forever. */
  return 0;
}
