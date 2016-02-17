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
void write_kernel(int handle, const void *buffer, int length)
{
  handle = handle;
  buffer = buffer;
  length = length;

  device_t *dev;
  gcd_t *gcd;

  dev = device_get(TYPECODE_TTY, 0);
  KERNEL_ASSERT(dev != NULL);

  gcd = (gcd_t *)dev->generic_device;
  KERNEL_ASSERT(gcd != NULL);

  gcd->write(gcd, buffer, length);
}
