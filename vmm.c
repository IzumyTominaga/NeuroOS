#include "vmm.h"
#include "pmm.h"
#include "vga.h"

__attribute__((aligned(4096))) static uint32_t kernel_page_directory[1024];
__attribute__((aligned(4096))) static uint32_t kernel_first_page_table[1024];

void vmm_init(void) {
    for (int i = 0; i < 1024; i++)
        kernel_page_directory[i] = 0 | VMM_FLAG_WRITE;

    for (uint32_t i = 0; i < 1024; i++) {
        uint32_t phys = i * PAGE_SIZE;
        kernel_first_page_table[i] = phys | VMM_FLAG_PRESENT | VMM_FLAG_WRITE;
    }
    kernel_page_directory[0] = ((uint32_t)kernel_first_page_table)
                               | VMM_FLAG_PRESENT | VMM_FLAG_WRITE;

    uint32_t pd = (uint32_t)kernel_page_directory;
    __asm__ volatile("mov %0, %%cr3" :: "r"(pd));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));

    vga_set_color(0x0A, 0x00);
    vga_puts("VMM: Paginacion MMU activada correctamente (CR0.PG = 1)\n");
}

void vmm_map(uint32_t* dir, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_idx = virt >> 22;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if (!(dir[pd_idx] & VMM_FLAG_PRESENT)) {
        uint32_t new_pt = pmm_alloc_frame();
        dir[pd_idx] = new_pt | VMM_FLAG_PRESENT | VMM_FLAG_WRITE | flags;
        uint32_t* pt = (uint32_t*)new_pt;
        for (int i = 0; i < 1024; i++) pt[i] = 0;
    }

    uint32_t* pt = (uint32_t*)(dir[pd_idx] & ~0xFFF);
    pt[pt_idx] = (phys & ~0xFFF) | flags;
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

uint32_t* vmm_create_directory(void) {
    uint32_t frame = pmm_alloc_frame();
    if (!frame) return 0;
    uint32_t* dir = (uint32_t*)frame;
    for (int i = 0; i < 1024; i++) dir[i] = 0;
    return dir;
}

void vmm_clone_kernel(uint32_t* new_dir) {
    new_dir[0] = kernel_page_directory[0];
    for (int i = 1; i < 1024; i++) new_dir[i] = 0;
}

void vmm_switch(uint32_t* dir) {
    __asm__ volatile("mov %0, %%cr3" :: "r"((uint32_t)dir) : "memory");
}

uint32_t* vmm_get_kernel_dir(void) {
    return kernel_page_directory;
}

static void print_hex(uint32_t n) {
    const char* h = "0123456789ABCDEF";
    char buf[9]; buf[8] = 0;
    for (int i = 7; i >= 0; i--) { buf[i] = h[n&0xF]; n>>=4; }
    vga_puts(buf);
}

static void print_num(uint32_t n) {
    char buf[12]; int i=10; buf[11]=0;
    do { buf[--i]='0'+(n%10); n/=10; } while(n&&i>0);
    vga_puts(buf+i);
}

void vmm_print_dir(uint32_t* dir) {
    vga_set_color(0x0E, 0x00);
    vga_puts("Page directory 0x"); print_hex((uint32_t)dir); vga_puts(":\n");
    int count = 0;
    for (int i = 0; i < 1024; i++) {
        if (dir[i] & VMM_FLAG_PRESENT) {
            vga_set_color(0x0A, 0x00);
            vga_puts("  ["); print_num(i); vga_puts("] -> 0x");
            print_hex(dir[i] & ~0xFFF);
            vga_puts(dir[i] & VMM_FLAG_USER ? " USR" : " KRN");
            vga_puts(dir[i] & VMM_FLAG_WRITE ? " RW" : " RO");
            vga_puts("\n");
            count++;
        }
    }
    if (!count) { vga_set_color(0x08, 0x00); vga_puts("  (vacio)\n"); }
}
