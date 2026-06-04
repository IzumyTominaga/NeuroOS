#ifndef IDT_H
#define IDT_H
#include <stdint.h>
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;
typedef struct {
    uint32_t ds;
    uint32_t edi,esi,ebp,esp,ebx,edx,ecx,eax;
    uint32_t int_no,err_code;
    uint32_t eip,cs,eflags,useresp,ss;
} regs_t;
void idt_init(void);
uint32_t timer_get_ticks(void);
uint8_t  kbd_get_last(void);
void     kbd_clear(void);
#endif
