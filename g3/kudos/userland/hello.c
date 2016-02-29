#include "lib.h"

int main(void) {
  puts("This is 'hello.mips32'.  I'll just sit here and busywait forever.\n");

  while (1); /* Loop forever. */
  return 0;
}
