/*
 * Process startup.
 */

#include <arch.h>

#include <stddef.h>             // NULL, comes from GCC.

#include "proc/process.h"
#include "proc/elf.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "fs/vfs.h"
#include "kernel/sleepq.h"
#include "vm/memory.h"

#include "drivers/device.h"     // device_*
#include "drivers/gcd.h"        // gcd_*
#include "kernel/assert.h"      // KERNEL_ASSERT
#include "proc/syscall.h"       // FILEHANDLE_*

/** @name Process startup
 *
 * This module contains functions for starting and managing userland
 * processes.
 */
extern void process_set_pagetable(pagetable_t*);

/* Return non-zero on error. */
int setup_new_process(TID_t thread,
                      const char *executable, const char **argv_src,
                      virtaddr_t *entry_point, virtaddr_t *stack_top)
{
  pagetable_t *pagetable;
  elf_info_t elf;
  openfile_t file;
  uintptr_t phys_page;
  int i, res;
  thread_table_t *thread_entry = thread_get_thread_entry(thread);

  int argc = 1;
  virtaddr_t argv_begin;
  virtaddr_t argv_dst;
  int argv_elem_size;
  virtaddr_t argv_elem_dst;

  file = vfs_open((char *)executable);

  /* Make sure the file existed and was a valid ELF file */
  if (file < 0) {
    return -1;
  }

  res = elf_parse_header(&elf, file);
  if (res < 0) {
    return -1;
  }

  /* Trivial and naive sanity check for entry point: */
  if (elf.entry_point < PAGE_SIZE) {
    return -1;
  }

  *entry_point = elf.entry_point;

  pagetable = vm_create_pagetable(thread);

  thread_entry->pagetable = pagetable;

  /* Allocate and map stack */
  for(i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
    phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);
    /* Zero the page */
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);
    vm_map(pagetable, phys_page,
           (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - i*PAGE_SIZE, 1);
  }

  /* Allocate and map pages for the segments. We assume that
     segments begin at page boundary. (The linker script in tests
     directory creates this kind of segments) */
  for(i = 0; i < (int)elf.ro_pages; i++) {
    int left_to_read = elf.ro_size - i*PAGE_SIZE;
    phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);
    /* Zero the page */
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);
    /* Fill the page from ro segment */
    if (left_to_read > 0) {
      KERNEL_ASSERT(vfs_seek(file, elf.ro_location + i*PAGE_SIZE) == VFS_OK);
      KERNEL_ASSERT(vfs_read(file, (void*)ADDR_PHYS_TO_KERNEL(phys_page),
                             MIN(PAGE_SIZE, left_to_read))
                    == (int) MIN(PAGE_SIZE, left_to_read));
    }
    vm_map(pagetable, phys_page,
           elf.ro_vaddr + i*PAGE_SIZE, 0);
  }

  for(i = 0; i < (int)elf.rw_pages; i++) {
    int left_to_read = elf.rw_size - i*PAGE_SIZE;
    phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);
    /* Zero the page */
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);
    /* Fill the page from rw segment */
    if (left_to_read > 0) {
      KERNEL_ASSERT(vfs_seek(file, elf.rw_location + i*PAGE_SIZE) == VFS_OK);
      KERNEL_ASSERT(vfs_read(file, (void*)ADDR_PHYS_TO_KERNEL(phys_page),
                             MIN(PAGE_SIZE, left_to_read))
                    == (int) MIN(PAGE_SIZE, left_to_read));
    }
    vm_map(pagetable, phys_page,
           elf.rw_vaddr + i*PAGE_SIZE, 1);
  }

  /* Set up argc and argv on the stack. */

  /* Start by preparing ancillary information for the new process argv. */
  if (argv_src != NULL)
    for (i = 0; argv_src[i] != NULL; i++) {
      argc++;
    }

  argv_begin = USERLAND_STACK_TOP - (argc * sizeof(virtaddr_t));
  argv_dst = argv_begin;

  /* Prepare for copying executable. */
  argv_elem_size = strlen(executable) + 1;
  argv_elem_dst = argv_dst - wordpad(argv_elem_size);

  /* Copy executable to argv[0] location. */
  vm_memwrite(pagetable,
              argv_elem_size,
              argv_elem_dst,
              executable);
  /* Set argv[i] */
  vm_memwrite(pagetable,
              sizeof(virtaddr_t),
              argv_dst,
              &argv_elem_dst);

  /* Move argv_dst to &argv[1]. */
  argv_dst += sizeof(virtaddr_t);

  if (argv_src != NULL) {
    for (i = 0; argv_src[i] != NULL; i++) {
      /* Compute the size of argv[i+1] */
      argv_elem_size = strlen(argv_src[i]) + 1;
      argv_elem_dst -= wordpad(argv_elem_size);

      /* Write the 'i+1'th element of argv */
      vm_memwrite(pagetable,
                  argv_elem_size,
                  argv_elem_dst,
                  argv_src[i]);

      /* Write argv[i+1] */
      vm_memwrite(pagetable,
                  sizeof(virtaddr_t),
                  argv_dst,
                  &argv_elem_dst);

      /* Move argv_dst to next element of argv. */
      argv_dst += sizeof(virtaddr_t);
    }
  }

  /* Write argc to the stack. */
  vm_memwrite(pagetable,
              sizeof(int),
              argv_elem_dst - sizeof(int),
              &argc);
  /* Write argv to the stack. */
  vm_memwrite(pagetable,
              sizeof(virtaddr_t),
              argv_elem_dst - sizeof(int) - sizeof(virtaddr_t),
              &argv_begin);

  /* Stack pointer points at argv. */
  *stack_top = argv_elem_dst - sizeof(int) - sizeof(virtaddr_t);

  return 0;
}

void process_start(const char *executable, const char **argv)
{
  TID_t my_thread;
  virtaddr_t entry_point;
  int ret;
  context_t user_context;
  virtaddr_t stack_top;

  my_thread = thread_get_current_thread();
  ret = setup_new_process(my_thread, executable, argv,
                          &entry_point, &stack_top);

  if (ret != 0) {
    return; /* Something went wrong. */
  }

  process_set_pagetable(thread_get_thread_entry(my_thread)->pagetable);

  /* Initialize the user context. (Status register is handled by
     thread_goto_userland) */
  memoryset(&user_context, 0, sizeof(user_context));

  _context_set_ip(&user_context, entry_point);
  _context_set_sp(&user_context, stack_top);

  thread_goto_userland(&user_context);
}

virtaddr_t process_entry_points[CONFIG_MAX_THREADS];
virtaddr_t process_stack_tops[CONFIG_MAX_THREADS];

void process_run()
{
  context_t user_context;

  TID_t thread_id =  thread_get_current_thread();
  thread_table_t *thread = thread_get_thread_entry(thread_id);

  process_set_pagetable(thread->pagetable);

  /* Initialize the user context. (Status register is handled by
     thread_goto_userland) */
  memoryset(&user_context, 0, sizeof(user_context));

  _context_set_ip(&user_context, process_entry_points[thread_id]);
  _context_set_sp(&user_context, process_stack_tops[thread_id]);

  thread_goto_userland(&user_context);
}

/* We don't care about join, so don't bother with a process table. */
int
process_spawn_silly(const char *executable, const char **argv)
{
  TID_t thread_id;
  int ret;

  thread_id = thread_create((void (*)(uint32_t))(&process_run), 0);
  ret = setup_new_process(thread_id, executable, argv,
                          &process_entry_points[thread_id],
                          &process_stack_tops[thread_id]);
  thread_run(thread_id);
  return ret;
}

/* When a process tries to exit, just complain and kill the thread. */
void
process_exit_silly()
{
  kwrite("SYSCALL_EXIT not supported in this version of KUDOS.\n");

  thread_table_t *thread = thread_get_current_thread_entry();
  vm_destroy_pagetable(thread->pagetable);
  thread->pagetable = NULL;
  thread_finish();
}

static int tty_read(void *buffer, int length) {
  device_t *dev;
  gcd_t *gcd;

  dev = device_get(TYPECODE_TTY, 0);
  if (dev == NULL) return IO_TTY_UNAVAILABLE;
  gcd = (gcd_t *)dev->generic_device;
  if (gcd == NULL) return IO_TTY_UNAVAILABLE;

  return gcd->read(gcd, buffer, length);
}

static int tty_write(const void *buffer, int length) {
  device_t *dev;
  gcd_t *gcd;

  dev = device_get(TYPECODE_TTY, 0);
  if (dev == NULL) return IO_TTY_UNAVAILABLE;
  gcd = (gcd_t *)dev->generic_device;
  if (gcd == NULL) return IO_TTY_UNAVAILABLE;

  return gcd->write(gcd, buffer, length);
}

int process_read(int filehandle, void *buffer, int length) {
  int retval = 0;

  if (length < 0) {
    retval = IO_NEGATIVE_LENGTH;
  } else if (!IN_USERLAND(buffer) || !IN_USERLAND(buffer + length)) {
    KERNEL_PANIC("SEGFAULT\n");
  } else if (filehandle == FILEHANDLE_STDIN) {
    retval = tty_read(buffer, length);
  } else if (filehandle == FILEHANDLE_STDOUT
            || filehandle == FILEHANDLE_STDERR) {
    retval = IO_INVALID_HANDLE;
  } else {
    retval = IO_NOT_IMPLEMENTED;
  }

  return retval;
}

int process_write(int filehandle, const void *buffer, int length) {
  int retval = 0;

  if (length < 0) {
    retval = IO_NEGATIVE_LENGTH;
  } else if (!IN_USERLAND(buffer) || !IN_USERLAND(buffer + length)) {
    KERNEL_PANIC("SEGFAULT\n");
  } else if (filehandle == FILEHANDLE_STDIN) {
    retval = IO_INVALID_HANDLE;
  } else if (filehandle == FILEHANDLE_STDOUT
            || filehandle == FILEHANDLE_STDERR) {
    retval = tty_write(buffer, length);
  } else {
    retval = IO_NOT_IMPLEMENTED;
  }

  return retval;
}
