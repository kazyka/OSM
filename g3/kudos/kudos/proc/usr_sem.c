#include "kernel/spinlock.h"
#include "lib/libc.h"
#include "proc/usr_sem.h"
#include "kernel/interrupt.h"
#include "kernel/sleepq.h"
#include "kernel/thread.h"

spinlock_t sem_table_slock;
static usr_sem_t usr_sem_table[MAX_SEMAPHORES];


usr_sem_t* usr_sem_open(const char* name, int value) {

  interrupt_status_t intr_status;
  intr_status = _interrupt_disable();

  spinlock_acquire(&sem_table_slock);

  if (value >= 0) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
      if (usr_sem_table[i].name != NULL) {
        if (!stringcmp(usr_sem_table[i].name, name)) {
          spinlock_release(&sem_table_slock);
          _interrupt_set_state(intr_status);
          return NULL;
        }
      }
    }

    for (int j = 0; j < MAX_SEMAPHORES; j++) {
      if (usr_sem_table[j].name == NULL) {
        stringcopy(usr_sem_table[j].name, name, 8);
        usr_sem_table[j].value = value;
        spinlock_release(&sem_table_slock);
        return &usr_sem_table[j];
      }
    }

    spinlock_release(&sem_table_slock);
    _interrupt_set_state(intr_status);
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
    _interrupt_set_state(intr_status);
    return NULL;
  }
}


int usr_sem_destroy(usr_sem_t* sem) {

  if (sem->value <= 0) {
    return -1; // fail, the semaphore is blocking
  }
  else {
    sem->value = 0;  // this might be hacky
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
