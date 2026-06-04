#include "shell.h"
#include <stdint.h>

static void sys_print_char(char c) {
    __asm__ volatile("mov $1, %%eax; mov %0, %%ebx; int $0x80" : : "r"((uint32_t)c) : "eax", "ebx");
}

static char sys_getchar(void) {
    uint32_t ret;
    __asm__ volatile("mov $2, %%eax; int $0x80; mov %%eax, %0" : "=r"(ret) : : "eax");
    return (char)ret;
}

static uint32_t sys_call1(uint32_t id, uint32_t arg) {
    uint32_t ret;
    __asm__ volatile("mov %1, %%eax; mov %2, %%ebx; int $0x80; mov %%eax, %0"
                     : "=r"(ret)
                     : "r"(id), "r"(arg)
                     : "eax", "ebx");
    return ret;
}

static uint32_t sys_call0(uint32_t id) {
    uint32_t ret;
    __asm__ volatile("mov %1, %%eax; int $0x80; mov %%eax, %0"
                     : "=r"(ret)
                     : "r"(id)
                     : "eax");
    return ret;
}

static void sys_print_string(const char* str) {
    while(*str) {
        sys_print_char(*str++);
    }
}

static int streq(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == 0 && b[i] == 0;
}

static int starts_with(const char* s, const char* prefix) {
    int i = 0;
    while (prefix[i]) {
        if (s[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

static uint32_t parse_u32(const char* s, int* ok) {
    uint32_t n = 0;
    int i = 0;
    *ok = 0;
    while (s[i] == ' ') i++;
    while (s[i] >= '0' && s[i] <= '9') {
        *ok = 1;
        n = (n * 10) + (uint32_t)(s[i] - '0');
        i++;
    }
    return n;
}

static void print_u32(uint32_t n) {
    char buf[12];
    int i = 10;
    buf[11] = 0;
    do {
        buf[--i] = (char)('0' + (n % 10));
        n /= 10;
    } while (n && i > 0);
    sys_print_string(buf + i);
}

static void run_command(char* line) {
    if (streq(line, "help")) {
        sys_print_string("Comandos: help, neuro, ps, ticks, clear, kill <pid>\n");
    } else if (streq(line, "neuro")) {
        sys_call0(SYS_NEURO);
    } else if (streq(line, "ps")) {
        sys_call0(SYS_PS);
    } else if (streq(line, "ticks")) {
        sys_print_string("Ticks: ");
        print_u32(sys_call0(SYS_TICKS));
        sys_print_char('\n');
    } else if (streq(line, "clear")) {
        sys_call0(SYS_CLEAR);
    } else if (starts_with(line, "kill ")) {
        int ok = 0;
        uint32_t pid = parse_u32(line + 5, &ok);
        if (!ok) {
            sys_print_string("Uso: kill <pid>\n");
        } else if (sys_call1(SYS_KILL, pid) == 0) {
            sys_print_string("Proceso terminado: ");
            print_u32(pid);
            sys_print_char('\n');
        } else {
            sys_print_string("No se pudo terminar ese proceso.\n");
        }
    } else {
        sys_print_string("Comando desconocido. Escribe help.\n");
    }
}

void task_shell(void) {
    sys_print_string("NeuroShell v1.3 (Ring 3) - Control del sistema\n");
    sys_print_string("NeuroShell> ");

    char line_buffer[64];
    int idx = 0;

    while(1) {
        char c = sys_getchar();
        
        if (c != 0) {
            if (c == '\n') {
                line_buffer[idx] = '\0';
                sys_print_char('\n');
                
                if (idx > 0) {
                    run_command(line_buffer);
                }
                
                idx = 0;
                sys_print_string("NeuroShell> ");
            } 
            else if (c == '\b') {
                if (idx > 0) {
                    idx--;
                    sys_print_char('\b');
                    sys_print_char(' ');
                    sys_print_char('\b');
                }
            } 
            else {
                if (idx < 63) {
                    line_buffer[idx++] = c;
                    sys_print_char(c);
                }
            }
        }
        
        __asm__ volatile("pause");
    }
}
