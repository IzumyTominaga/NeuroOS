#include <stdint.h>
#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "pmm.h"
#include "vmm.h"
#include "proc.h"
#include "syscall.h"
#include "shell.h"
#include "keyboard.h"
#include "neuro.h"

typedef struct {
    uint32_t flags;
    uint32_t mem_lower, mem_upper;
    uint32_t boot_device, cmdline;
    uint32_t mods_count, mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length, mmap_addr;
} __attribute__((packed)) multiboot_info_t;

#define MULTIBOOT_MAGIC 0x2BADB002
#define MB_FLAG_MMAP    (1<<6)

void pit_init(uint32_t hz) {
    uint32_t divisor = 1193180 / hz;
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x36), "Nd"((uint16_t)0x43));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(divisor & 0xFF)), "Nd"((uint16_t)0x40));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)((divisor >> 8) & 0xFF)), "Nd"((uint16_t)0x40));
}

void task_b(void) {
    volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
    while(1) {
        vga_buffer[79] = (0x0C << 8) | '*';
        for(volatile int i = 0; i < 400000; i++);
        vga_buffer[79] = (0x00 << 8) | ' ';
        for(volatile int i = 0; i < 400000; i++);
    }
}

void task_c(void) {
    while(1) {
        __asm__ volatile("pause");
    }
}

void kernel_main(uint32_t magic, multiboot_info_t* mb) {
    vga_init();
    vga_set_color(0x0F, 0x00);
    vga_puts("NeuroOS - Multitarea Apropiativa (Preemptive)\n");
    vga_set_color(0x08, 0x00);
    vga_puts("---------------------------------------------\n");

    gdt_init();
    idt_init();

    if (magic == MULTIBOOT_MAGIC && (mb->flags & MB_FLAG_MMAP)) {
        pmm_init(mb->mmap_addr, mb->mmap_length);
        pmm_print_stats();
    }
    vmm_init();

    syscall_init();
    proc_init();
    keyboard_init();

    vga_set_color(0x0E, 0x00);
    vga_puts("Iniciando PIT a 100Hz...\n");
    pit_init(100);

    vga_set_color(0x0B, 0x00);
    vga_puts("Creando procesos en Ring 3...\n");
    neuro_init(100);
    proc_create("Shell", task_shell, 2);
    proc_create("Bkg_Task", task_b, 1);
    proc_create("Dummy", task_c, 1);

    vga_set_color(0x0F, 0x00);
    vga_puts("\n[ Memoria virtual por proceso ]\n");
    vga_set_color(0x08, 0x00);
    vga_puts("Cada proceso tiene su propio page directory.\n");
    vga_puts("Mismo virt addr -> distinta fisica por proceso.\n\n");
    vga_set_color(0x0A, 0x00);
    vga_puts("Lanzando Planificador hacia Ring 3...\n");

    __asm__ volatile("sti");
    proc_yield();

    while(1) {
        __asm__ volatile("hlt");
    }
}
