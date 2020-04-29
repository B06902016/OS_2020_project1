#ifndef _PROCESS_CONTROL_H_
#define _PROCESS_CONTROL_H_

#include <sys/types.h>

typedef struct{
    char name[32];
    int ready_time;
    int exec_time;
    pid_t pid;
    int pipe_fd[2];
}Process;

// defines one unit of time
void TIME_UNIT(void);

// core: 0 for scheduler, 1 for child processes
int assign_core(pid_t pid, int core);

// spawn new process when ready_time is reached
pid_t proc_create(Process chld);

// kick child out of core
int proc_kickout(pid_t pid);

// bring another child to run
int proc_resume(pid_t pid);

#endif
