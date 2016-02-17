/*
 * Read from kernel
 */
#include "kernel/write.h"
#include "drivers/metadev.h"
#include "lib/libc.h"
#include "fs/vfs.h"

#include "drivers/device.h"
#include "drivers/gcd.h"
#include "kernel/assert.h"

/**
 * Read from kernel.
 */
int read_kernel(int handle, void *buffer, int length)
{
  KERNEL_ASSERT(handle == 0);
  KERNEL_ASSERT(length >= strlen(buffer));

  device_t *dev;
  gcd_t *gcd;
  int buf_len;
  buf_len = 0;

  dev = device_get(TYPECODE_TTY, 0);
  KERNEL_ASSERT(dev != NULL);

  gcd = (gcd_t *)dev->generic_device;
  KERNEL_ASSERT(gcd != NULL);

  while(stringcmp(buffer, "n") != 0){

    buf_len = buf_len + gcd->read(gcd, buffer, length);
    KERNEL_ASSERT(buf_len >= 0);
    kwrite(buffer);
  }
  gcd->write(gcd, buffer, buf_len);
  return buf_len;
}
