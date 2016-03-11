/*
 * TLB handling
 */

#include "lib/libc.h"
#include "kernel/thread.h"
#include "kernel/panic.h"
#include "kernel/assert.h"
#include "proc/process.h"
#include <tlb.h>
#include <pagetable.h>

#define EXCEPTION_USERLAND 0
#define EXCEPTION_KERNEL 1

int search_pagetable(pagetable_t* pagetable, int* entry_index)
{
  // Search pagetable for matching entry belonging
  // to the thread, that caused the exception.

  tlb_exception_state_t tes;
  _tlb_get_exception_state(&tes);

  for (int i = 0; i < PAGETABLE_ENTRIES; i++){
    if (tes.badvpn2 == pagetable->entries[i].VPN2){
      *entry_index = i;
      return 0; // Found a match
    }
  }

  return -1; // Did not find a match
}

void tlb_modified_exception(int exception_origin)
{
  int dummy_val = 3;

  switch (exception_origin) {
  case EXCEPTION_USERLAND:
    process_exit(dummy_val);
    break;
  case EXCEPTION_KERNEL:
    KERNEL_PANIC("TLB Modify: Kernel panic");
    break;
  default:
    KERNEL_PANIC("TLB Modify: Unhandled function argument");
    break;
  }
}

void tlb_load_exception(int exception_origin)
{
  int search_pagetable_result, entry_index;
  pagetable_t* pagetable = thread_get_current_thread_entry()->pagetable;

  search_pagetable_result = search_pagetable(pagetable,&entry_index);
  int dummy_val = 3;

  if (search_pagetable_result == 0) {
    tlb_entry_t entry = pagetable->entries[entry_index];
    _tlb_write_random(&entry);
  }
  else if (search_pagetable_result == -1) {
      switch (exception_origin) {
      case EXCEPTION_USERLAND:
        process_exit(dummy_val);
        break;
      case EXCEPTION_KERNEL:
        KERNEL_PANIC("TLB Load: Kernel panic");
        break;
      default:
        KERNEL_PANIC("TLB Load: Unhandled function argument");
        break;
      }
  }
  else
    KERNEL_PANIC("TLB Load: Unknown return value from search_pagetable");
}

void tlb_store_exception(int exception_origin)
{
  // The load and store exceptions are handled the same
  tlb_load_exception(exception_origin);
}


/**
 * Fill TLB with given pagetable. This function is used to set memory
 * mappings in CP0's TLB before we have a proper TLB handling system.
 * This approach limits the maximum mapping size to 128kB.
 *
 * @param pagetable Mappings to write to TLB.
 *
 */

void tlb_fill(pagetable_t *pagetable)
{
  if(pagetable == NULL)
    return;

  /* Check that the pagetable can fit into TLB. This is needed until
     we have proper VM system, because the whole pagetable must fit
     into TLB. */
  KERNEL_ASSERT(pagetable->valid_count <= (_tlb_get_maxindex()+1));

  _tlb_write(pagetable->entries, 0, pagetable->valid_count);

  /* Set ASID field in Co-Processor 0 to match thread ID so that
     only entries with the ASID of the current thread will match in
     the TLB hardware. */
  _tlb_set_asid(pagetable->ASID);
}
