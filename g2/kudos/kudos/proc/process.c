/*
 * Process startup.
 */

#include <arch.h>
#include "proc/process.h"
#include "proc/elf.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "kernel/spinlock.h"
#include "kernel/interrupt.h"
#include "fs/vfs.h"
#include "kernel/sleepq.h"
#include "vm/memory.h"


/** @name Process startup
 *
 * This module contains functions for starting and managing userland
 * processes.
 */

extern void process_set_pagetable(pagetable_t*);

process_control_block_t process_table[PROCESS_MAX_PROCESSES];

spinlock_t process_table_slock;

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


process_id_t process_spawn(const char *executable, const char **argv)
{
  TID_t new_thread;
  virtaddr_t entry_point;
  int ret;
  virtaddr_t stack_top;
  process_id_t PID;
  interrupt_status_t intr_status;

  PID = first_free_PID();
  if (PID == ERROR_NO_FREE_PID){
    return -1; // error: process_table full
  }

  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  process_table[PID].state = PROCESS_RUNNING;
  thread_get_current_thread_entry()->process_id = PID;

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  new_thread = thread_create(&process_run, PID);

  ret = setup_new_process(new_thread, executable, argv,
                          &entry_point, &stack_top);
  if (ret != 0) {
    return -1; /* Something went wrong. */
  }

  process_table[PID].entry_point = entry_point;
  process_table[PID].stack_top = stack_top;
  thread_run(new_thread);
  return 0;
}

void process_run(process_id_t PID){
  TID_t my_thread;
  virtaddr_t entry_point;
  context_t user_context;
  virtaddr_t stack_top;

  entry_point = process_table[PID].entry_point;
  stack_top = process_table[PID].stack_top;
  my_thread = thread_get_current_thread();

  process_set_pagetable(thread_get_thread_entry(my_thread)->pagetable);

  /* Initialize the user context. (Status register is handled by
     thread_goto_userland) */
  memoryset(&user_context, 0, sizeof(user_context));

  _context_set_ip(&user_context, entry_point);
  _context_set_sp(&user_context, stack_top);

  thread_goto_userland(&user_context);
}


void process_init(void)
{
  spinlock_reset(&process_table_slock);

  /* Init the entries of process_table */
  for (int i=0; i<PROCESS_MAX_PROCESSES; i++) {
    process_table[i].state        = PROCESS_FREE;
    process_table[i].process_id   = i;
  }
}

process_id_t first_free_PID(void){

  // Ensuring mutual exclusion when acessing the process_table
  interrupt_status_t intr_status;
  process_id_t PID = -1;
  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);
  for (int i=0; i<PROCESS_MAX_PROCESSES; i++) {
    if (process_table[i].state == PROCESS_FREE)
      PID = process_table[i].process_id;
      spinlock_release(&process_table_slock);
      _interrupt_set_state(intr_status);
      return PID;
  }
  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);
  // No "FREE" process available
  return ERROR_NO_FREE_PID;
}

process_id_t process_get_current_process(void){
  return thread_get_current_thread_entry()->process_id;
}

int process_join(process_id_t pid){
  int retval;
  interrupt_status_t intr_status;
  intr_status = _interrupt_disable();
  spinlock_acquire(&process_table_slock);

  while (process_table[pid].state != PROCESS_ZOMBIE){
    sleepq_add(&process_table[pid].state);

    spinlock_release(&process_table_slock);

    thread_switch();

    spinlock_acquire(&process_table_slock);
  }
  process_table[pid].state = PROCESS_FREE;
  retval = process_table[pid].retval;

  spinlock_release(&process_table_slock);
  _interrupt_set_state(intr_status);

  return retval;

}

void process_exit(int retval){
  retval = retval;
  process_id_t PID = process_get_current_process();
  process_table[PID].state = PROCESS_ZOMBIE;
  thread_table_t thr = *thread_get_current_thread_entry();
  vm_destroy_pagetable(thr.pagetable);
  thr.pagetable = NULL;
  thread_finish();
}
