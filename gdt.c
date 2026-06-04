#include "gdt.h"

/* Incrementamos a 6 entradas para hacer espacio al TSS */
static gdt_entry_t gdt[6];
static gdt_ptr_t   gdt_ptr;
static tss_entry_t tss;

/* Stack seguro dedicado de 4KB para el handler de syscalls */
static uint8_t tss_esp0_stack[4096] __attribute__((aligned(16)));

extern void gdt_flush(uint32_t);

static void gdt_set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].base_low  = base & 0xFFFF;
    gdt[i].base_mid  = (base >> 16) & 0xFF;
    gdt[i].base_high = (base >> 24) & 0xFF;
    gdt[i].limit_low = limit & 0xFFFF;
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access    = access;
}

void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    /* Limpiar la estructura del TSS en memoria */
    uint8_t* tss_ptr = (uint8_t*)&tss;
    for(int i = 0; i < sizeof(tss_entry_t); i++) tss_ptr[i] = 0;

    /* Configurar el TSS: decirle al CPU donde esta el stack seguro (Ring 0) */
    tss.ss0  = 0x10; /* Selector de datos del kernel (GDT[2]) */
    tss.esp0 = (uint32_t)(tss_esp0_stack + 4096); /* Tope del stack */
    tss.iomap_base = sizeof(tss_entry_t); /* Deshabilitar I/O bitmap */

    gdt_set(0, 0, 0x00000000, 0x00, 0x00); /* Nulo */
    gdt_set(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* KCode */
    gdt_set(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* KData */
    gdt_set(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* UCode (Ring 3) */
    gdt_set(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* UData (Ring 3) */
    
    /* GDT[5]: TSS. Acceso 0x89 (Presente, Ring0, 32-bit TSS) */
    gdt_set(5, (uint32_t)&tss, sizeof(tss_entry_t) - 1, 0x89, 0x00);

    gdt_flush((uint32_t)&gdt_ptr);

    /* Informar al CPU donde esta el Task Register (Indice 5 * 8 bytes = 0x28) */
    uint16_t tr = 0x28;
    __asm__ volatile("ltr %0" : : "r" (tr));
}
