/*
 * Basic test of `syscall_memlimit`.
 *
 * Tests that `syscall_memlimit` can be used to extend the heap end, and that
 * the returned address range can be used for writing and reading.
 *
 * THIS TEST IS NOT ENOUGH.  You must also make a test of your own.  In
 * particular, this test only calls `sycall_memlimit` once, which doesn't test a
 * lot.
 *
 * This test prints
 *
 *     Nice string
 *
 * if your implementation works.
 */

#include "lib.h"

#define STRING_LENGTH 12
#define STRING_SOURCE "Nice string\n"

int main() {
  char* a_string;

  /* Find the current (initial) heap end. */
  a_string = syscall_memlimit(NULL);

  /* Extend the heap, and check that it worked. */
  if (syscall_memlimit(a_string + STRING_LENGTH) == NULL) {
    /* It didn't work, so exit already. */
    return 1;
  }

  /* Copy the source string to the dynamically allocated memory. */
  for (size_t i = 0; i < STRING_LENGTH; i++) {
    a_string[i] = STRING_SOURCE[i];
  }

  /* Write from the dynamically allocated memory. */
  syscall_write(1, a_string, STRING_LENGTH);

  return 0;
}
