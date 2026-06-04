#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "proc.h"

#define SYS_WRITE    1
#define SYS_READ     2
#define SYS_EXIT     3
#define SYS_GETPID   4
#define SYS_GETPROCS  5
#define SYS_KILL     6
#define SYS_NEURO    7
#define SYS_PS       8
#define SYS_TICKS    9
#define SYS_CLEAR    10

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} registers_t;

void syscall_init(void);
void syscall_handler_c(registers_t* regs);

#endif
