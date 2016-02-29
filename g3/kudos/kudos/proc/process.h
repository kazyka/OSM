/*
 * Process startup.
 */

#ifndef KUDOS_PROC_PROCESS
#define KUDOS_PROC_PROCESS

#include "lib/types.h"
#include "vm/memory.h"

#define PROCESS_PTABLE_FULL  -1
#define PROCESS_ILLEGAL_JOIN -2

#define PROCESS_MAX_FILELENGTH 256
#define PROCESS_MAX_PROCESSES  128
#define PROCESS_MAX_FILES      10

typedef int process_id_t;

void process_start(const char *executable, const char **argv);

int process_read(int filehandle, void *buffer, int length);

int process_write(int filehandle, const void *buffer, int length);

int process_spawn_silly(const char *executable, const char **argv);

void process_exit_silly(void);

#endif
