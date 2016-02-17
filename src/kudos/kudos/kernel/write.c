/*
 * Write from kernel
 */
#include "kernel/write.h"
#include "drivers/metadev.h"
#include "lib/libc.h"
#include "fs/vfs.h"

#include "drivers/device.h"
#include "drivers/gcd.h"
#include "kernel/assert.h"

/**
 * Write from kernel.
 */
int write_kernel(int handle, const void *buffer, int length)
{
  KERNEL_ASSERT((handle == 1)||(handle == 2));
  KERNEL_ASSERT(length == strlen(buffer));

  device_t *dev;
  gcd_t *gcd;

  dev = device_get(TYPECODE_TTY, 0);
  KERNEL_ASSERT(dev != NULL);

  gcd = (gcd_t *)dev->generic_device;
  KERNEL_ASSERT(gcd != NULL);

  gcd->write(gcd, buffer, length);

  return length;
}
