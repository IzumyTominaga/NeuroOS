#include "proc.h"
#include "vga.h"
#include "vmm.h"
#include "neuro.h"
#include <stdint.h>

static volatile uint32_t system_ticks = 0;
uint32_t get_system_ticks(void) { return system_ticks; }

static process_t procs[MAX_PROCS];
static int        current_pid = 0;
static int        proc_count  = 0;

extern void context_switch(context_t* old, context_t* new);

void proc_init(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        procs[i].state = PROC_DEAD;
        procs[i].pid   = i;
        procs[i].ticks = 0;
    }
    procs[0].state         = PROC_RUNNING;
    procs[0].priority      = 0;
    procs[0].base_priority = 0;
    procs[0].page_dir      = vmm_get_kernel_dir();
    procs[0].name[0] = 'K'; procs[0].name[1] = 'R'; procs[0].name[2] = 'N'; procs[0].name[3] = 'L'; procs[0].name[4] = 0;
    proc_count  = 1;
    current_pid = 0;
}

void proc_create(const char* name, void (*entry)(void), uint32_t priority) {
    if (proc_count >= MAX_PROCS) return;

    int slot = -1;
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state == PROC_DEAD) { slot = i; break; }
    }
    if (slot < 0) return;

    process_t* p = &procs[slot];

    int i = 0;
    while (name[i] && i < 15) { p->name[i] = name[i]; i++; }
    p->name[i] = 0;

    p->priority      = priority;
    p->base_priority = priority;
    p->ticks         = 0;
    p->state         = PROC_READY;
    p->page_dir = vmm_create_directory();
    if (p->page_dir) vmm_clone_kernel(p->page_dir);

    uint32_t* stack = (uint32_t*)&p->stack[PROC_STACK_SIZE/4];

    stack--; *stack = 0x23;
    stack--; *stack = (uint32_t)stack;
    stack--; *stack = 0x202;
    stack--; *stack = 0x1B;
    stack--; *stack = (uint32_t)entry;

    p->ctx.esp    = (uint32_t)stack;
    p->ctx.eip    = (uint32_t)entry;
    p->ctx.eflags = 0x202;
    p->ctx.cs     = 0x1B;
    p->ctx.eax = p->ctx.ebx = p->ctx.ecx = p->ctx.edx = 0;
    p->ctx.esi = p->ctx.edi = p->ctx.ebp = 0;

    proc_count++;
}

void proc_schedule(void) {
    int start = current_pid;
    int next  = -1;
    int best_prio = -1;

    for (int i = 1; i <= MAX_PROCS; i++) {
        int idx = (start + i) % MAX_PROCS;
        if (idx == 0 && proc_count > 1) continue;

        if (procs[idx].state == PROC_READY && (int)procs[idx].priority >= best_prio) {
            best_prio = procs[idx].priority;
            next = idx;
        }
    }

    if (next < 0) {
        if (procs[0].state == PROC_READY || procs[0].state == PROC_RUNNING) next = 0;
        else return;
    }

    if (next == current_pid) return;

    process_t* old = &procs[current_pid];
    process_t* new = &procs[next];

    if (old->state == PROC_RUNNING)
        old->state = PROC_READY;

    new->state  = PROC_RUNNING;
    new->ticks++;
    current_pid = next;
    new->priority = new->base_priority;

    if (new->page_dir)
        vmm_switch(new->page_dir);

    context_switch(&old->ctx, &new->ctx);
}

void proc_yield(void) {
    proc_schedule();
}

process_t* proc_current(void) {
    return &procs[current_pid];
}

static void print_num(uint32_t n) {
    char buf[12]; int i = 10; buf[11] = 0;
    do { buf[--i] = '0' + (n%10); n /= 10; } while (n && i > 0);
    vga_puts(buf + i);
}

void proc_print_list(void) {
    vga_set_color(0x0E, 0x00);
    vga_puts("PID  NOMBRE          ESTADO   TICKS  PRIO\n");
    vga_set_color(0x08, 0x00);
    vga_puts("---  ------          ------   -----  ----\n");
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state == PROC_DEAD) continue;
        const char* states[] = {"DEAD   ","READY  ","RUNNING","BLOCKED"};
        vga_set_color(procs[i].state == PROC_RUNNING ? 0x0A : 0x0F, 0x00);
        vga_puts("  ");
        print_num(procs[i].pid);
        vga_puts("  ");
        int j = 0;
        while (procs[i].name[j]) { char s[2]={procs[i].name[j],0}; vga_puts(s); j++; }
        while (j++ < 14) vga_puts(" ");
        vga_puts("  ");
        vga_puts(states[procs[i].state]);
        vga_puts("  ");
        print_num(procs[i].ticks);
        vga_puts("      ");
        print_num(procs[i].priority);
        vga_puts("\n");
    }
}

int proc_get_info_all(proc_info_t* buf, int max_entries) {
    int count = 0;
    for (int i = 0; i < MAX_PROCS; i++) {
        if (count >= max_entries) break;

        buf[count].pid      = procs[i].pid;
        buf[count].state    = (uint32_t)procs[i].state;
        buf[count].ticks    = procs[i].ticks;
        buf[count].priority = procs[i].priority;

        int j = 0;
        if (procs[i].state != PROC_DEAD) {
            while (procs[i].name[j] && j < 15) {
                buf[count].name[j] = procs[i].name[j];
                j++;
            }
        } else {
            buf[count].name[0] = '-'; j = 1;
        }
        buf[count].name[j] = '\0';

        count++;
    }
    return count;
}

int proc_kill(uint32_t pid) {
    if (pid == 0) return -1;

    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].state != PROC_DEAD && procs[i].pid == pid) {
            procs[i].state = PROC_DEAD;
            
            if (proc_count > 1) {
                proc_count--;
            }

            if (current_pid == (int)pid) {
                proc_schedule();
            }
            return 0;
        }
    }
    return -1;
}

void proc_boost_priority(uint32_t pid, uint32_t new_prio) {
    for (int i = 0; i < MAX_PROCS; i++) {
        if (procs[i].pid == pid && procs[i].state != PROC_DEAD) {
            procs[i].priority = new_prio;
            return;
        }
    }
}

void tick_and_schedule(void) {
    system_ticks++;
    neuro_tick();
    proc_schedule();
}
