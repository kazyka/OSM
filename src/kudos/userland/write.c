/*
 * testing for write syscalls
 */

#include "lib.h"

int main(void) {
  void *test_text = "Testing write\n";
  syscall_write(1, test_text, 14);
  syscall_write(0, test_text, 15); //causes kernel panic
  syscall_write(1, test_text, 14);
  syscall_halt();
  return 0;
}
