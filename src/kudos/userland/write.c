/*
 * testing for read and write syscalls
 * int syscall_read(int filehandle, void *buffer, int length);
 * int syscall_write(int filehandle, const void *buffer, int length);
 */

#include "lib.h"

int main(void) {
  void *test_text = "Testing write\n";
  syscall_write(1, test_text, 15);
  syscall_halt();
  return 0;
}
