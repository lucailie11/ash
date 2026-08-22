#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define CHILD_PID 0
#define EXEC_INTERNAL_ERROR -1

int process_spawn(char **argv);
int exit_code_from_status(int status);
pid_t wait_for_pid(pid_t pid, int *status);

#endif
