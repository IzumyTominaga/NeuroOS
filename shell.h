#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "proc.h"

#define SYS_WRITE    1
#define SYS_READ     2
#define SYS_EXIT     3
#define SYS_GETPID   4
#define SYS_GETPROCS 5
#define SYS_KILL     6
#define SYS_NEURO    7
#define SYS_PS       8
#define SYS_TICKS    9
#define SYS_CLEAR    10

void task_shell(void);

#endif
