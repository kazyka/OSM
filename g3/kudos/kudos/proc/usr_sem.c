#include "kernel/spinlock.h"
#include "lib/libc.h"
#include "proc/usr_sem.h"
#include "kernel/interrupt.h"
#include "kernel/sleepq.h"
#include "kernel/thread.h"

spinlock_t sem_table_slock;
static usr_sem_t usr_sem_table[MAX_SEMAPHORES];


usr_sem_t* usr_sem_open(const char* name, int value) {

  spinlock_acquire(&sem_table_slock);

  if (value >= 0) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
      if (usr_sem_table[i].name != NULL) {
        if (!stringcmp(usr_sem_table[i].name, name)) {
          kprintf(usr_sem_table[i].name);
          spinlock_release(&sem_table_slock);
          return NULL;
        }
      }
    }

    for (int j = 0; j < MAX_SEMAPHORES; j++) {
      if (usr_sem_table[j].name == NULL) {
        usr_sem_table[j].name = name;
        usr_sem_table[j].value = value;
        kprintf("Value of j: %d\n",j);
        kprintf("Value of value: %d\n",value);
        kprintf("Value of sem value: %d\n",usr_sem_table[j].value);
        spinlock_release(&sem_table_slock);
        kprintf("Value of j: %d\n",j);
        kprintf("Value of value: %d\n",value);
        kprintf("Value of sem value: %d\n",usr_sem_table[j].value);
        return &usr_sem_table[j];
      }
    }

    spinlock_release(&sem_table_slock);
    return NULL;
  }
  else {
    for (int k = 0; k < MAX_SEMAPHORES; k++) {
      if (stringcmp(usr_sem_table[k].name, name)) {

        spinlock_release(&sem_table_slock);
        return &usr_sem_table[k]; // return handle already in use
      }
    }
    spinlock_release(&sem_table_slock);
    return NULL;
  }
}


int usr_sem_destroy(usr_sem_t* sem) {
  kprintf("1\n");
  int test = sem->value;
  kprintf("Test value is: %d\n",test);
  if (sem->value <= 0) {
    kprintf("2\n");
    return -1; // fail, the semaphore is blocking
  }
  else {
    kprintf("4\n");
    sem->value = 0;  // this might be hacky
    kprintf("5\n");
    sem->name = NULL;
    return 0;
  }
}

int usr_sem_procure(usr_sem_t* sem) {
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&(sem->sem_slock));

  sem->value--;
  while (sem->value < 0) {
    sleepq_add(&(sem->value));
    spinlock_release(&(sem->sem_slock));
    thread_switch();
    spinlock_acquire(&(sem->sem_slock));
  }

  spinlock_release(&(sem->sem_slock));
  _interrupt_set_state(intr_status);
  return 0;
}

int usr_sem_vacate(usr_sem_t* sem) {
  interrupt_status_t intr_status;

  intr_status = _interrupt_disable();
  spinlock_acquire(&(sem->sem_slock));

  sem->value++;
  if (sem->value <= 0) {
    sleepq_wake(&(sem->value));
  }

  spinlock_release(&(sem->sem_slock));
  _interrupt_set_state(intr_status);
  return 0;
}
