#define MAX_SEMAPHORES 128

typedef struct {
  const char* name;
  int value;
  spinlock_t sem_slock;
} usr_sem_t;


usr_sem_t* usr_sem_open(const char* name, int value);
int usr_sem_destroy(usr_sem_t* sem);
int usr_sem_procure(usr_sem_t* sem);
int usr_sem_vacate(usr_sem_t* sem);


