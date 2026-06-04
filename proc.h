#ifndef PROC_H
#define PROC_H
#include <stdint.h>

#define PROC_STACK_SIZE 4096
#define MAX_PROCS       8

typedef enum {
    PROC_DEAD    = 0,
    PROC_READY   = 1,
    PROC_RUNNING = 2,
    PROC_BLOCKED = 3
} proc_state_t;

typedef struct {
    uint32_t edi, esi, ebp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags;
    uint32_t esp;
} context_t;

typedef struct {
    uint32_t    pid;
    proc_state_t state;
    context_t   ctx;
    uint32_t    stack[PROC_STACK_SIZE / 4];
    char        name[16];
    uint32_t    ticks;
    uint32_t    priority;
    uint32_t    base_priority;
    uint32_t*   page_dir;
} process_t;

typedef struct {
    uint32_t pid;
    uint32_t state;
    uint32_t ticks;
    uint32_t priority;
    char     name[16];
} proc_info_t;

void proc_init(void);
void proc_create(const char* name, void (*entry)(void), uint32_t priority);
void proc_schedule(void);
void proc_yield(void);
process_t* proc_current(void);
void proc_print_list(void);

int proc_get_info_all(proc_info_t* buf, int max_entries);
int proc_kill(uint32_t pid);
uint32_t get_system_ticks(void);
void tick_and_schedule(void);
void proc_boost_priority(uint32_t pid, uint32_t new_prio);

#endif
