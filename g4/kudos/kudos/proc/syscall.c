/*
 * System calls.
 */
#include <cswitch.h>
#include "proc/syscall.h"
#include "kernel/halt.h"
#include "kernel/panic.h"
#include "lib/libc.h"
#include "kernel/assert.h"
#include "vm/memory.h"
#include "drivers/polltty.h"
#include "proc/process.h"
#include "kernel/thread.h"
#include "vm/memory.h"
#include <arch.h>

#define PAGE_SIZE 4096

int syscall_write(const char *buffer, int length) {
  /* Not a G1 solution! */
  for (int i = 0; i < length; i++, *buffer++) polltty_putchar(*buffer);
  return length;
}

int syscall_read(char *buffer) {
  /* Not a G1 solution! */
  *buffer = polltty_getchar();
  return 1;
}

void* syscall_memlimit(void* new_end){

  void* heap_end = process_get_current_process_entry()->heap_end;
  pagetable_t* pagetable = thread_get_current_thread_entry()->pagetable;

  // Return the current heap_end if new_end is NULL
  // OR if heap_end is greater than or equal to new_end
  // (trying to shrink the heap)

  if (new_end == NULL || heap_end >= new_end){
    return heap_end;
  }

  int number_of_pages = ((int) new_end -(int) heap_end) / PAGE_SIZE + 1;

  for (int i = 0; i < number_of_pages; ++i){
    uint32_t vaddr = ((int) heap_end) + PAGE_SIZE * (i + 1);
    uintptr_t phys_page = physmem_allocblock();
    KERNEL_ASSERT(phys_page != 0);

    // Zero the page
    memoryset((void*)ADDR_PHYS_TO_KERNEL(phys_page), 0, PAGE_SIZE);

    vm_map(pagetable, phys_page, vaddr, 1);
  }


  // Update the current heap_end to new_end

  heap_end = new_end;

  return heap_end;
}

/**
 * Handle system calls. Interrupts are enabled when this function is
 * called.
 */
uintptr_t syscall_entry(uintptr_t syscall,
                        uintptr_t arg0, uintptr_t arg1, uintptr_t arg2)
{
  arg0 = arg0;
  arg1 = arg1;
  arg2 = arg2;
  /* When a syscall is executed in userland, register a0 contains
   * the number of the syscall. Registers a1, a2 and a3 contain the
   * arguments of the syscall. The userland code expects that after
   * returning from the syscall instruction the return value of the
   * syscall is found in register v0. Before entering this function
   * the userland context has been saved to user_context and after
   * returning from this function the userland context will be
   * restored from user_context.
   */
  switch(syscall) {
  case SYSCALL_HALT:
    halt_kernel();
    break;
  case SYSCALL_READ:
    return syscall_read((void*)arg1);
    break;
  case SYSCALL_WRITE:
    return syscall_write((const void*)arg1, (int)arg2);
    break;
  case SYSCALL_SPAWN:
    return process_spawn((char*) arg0, (char const**) arg1);
    break;
  case SYSCALL_EXIT:
    process_exit((process_id_t) arg0);
    break;
  case SYSCALL_JOIN:
    return process_join((process_id_t) arg0);
    break;
  case SYSCALL_MEMLIMIT:
    syscall_memlimit((void *) arg0);
    break;
  default:
    KERNEL_PANIC("Unhandled system call\n");
  }

  return 0;
}
