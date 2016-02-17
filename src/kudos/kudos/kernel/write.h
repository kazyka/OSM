/*
 * Write from kernel
 */

#ifndef KUDOS_KERNEL_WRITE_H
#define KUDOS_KERNEL_WRITE_H

int write_kernel(int handle, const void *buffer, int length);

#endif /* KUDOS_KERNEL_WRITE_H */
