#include "syscall.h"
#include "vga.h"
#include "proc.h"
#include "keyboard.h"
#include "neuro.h"

void syscall_handler_c(registers_t* regs) {
    switch(regs->eax) {
        case 1:
            {
                char str[2] = {(char)regs->ebx, '\0'};
                vga_puts(str);
            }
            break;
            
        case 2:
            regs->eax = (uint32_t)keyboard_getchar();
            break;
            
        case 3:
            vga_puts("\n[Proceso finalizado]\n");
            while(1) { __asm__ volatile("hlt"); }
            break;

        case 4:
            regs->eax = proc_current()->pid;
            break;

        case 6:
            regs->eax = (uint32_t)proc_kill(regs->ebx);
            break;

        case 7:
            neuro_print_status();
            break;
        case 8:
            proc_print_list();
            break;
        case 9:
            regs->eax = get_system_ticks();
            break;
        case 10:
            vga_clear();
            break;
        default:
            vga_puts("[syscall invalida]\n");
            break;
    }
}

void syscall_init(void) {
    /* Tabla inicializada estaticamente, nada que hacer */
}
