#include "keyboard.h"
#include "vga.h"

#define KB_DATA_PORT 0x60

#define BUFFER_SIZE 256
static char kb_buffer[BUFFER_SIZE];
static int kb_head = 0;
static int kb_tail = 0;

/* Tabla de traduccion Scancode a ASCII (Teclado US estandar simplificado) */
static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  '-',   0,   0,   0, '+',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_init(void) {
    kb_head = 0;
    kb_tail = 0;
}

void keyboard_handler(void) {
    uint8_t scancode;
    __asm__ volatile("inb %1, %0" : "=a"(scancode) : "Nd"((uint16_t)KB_DATA_PORT));
    /* DEBUG: marcar en esquina de pantalla que IRQ1 llego */
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    vga[79] = (uint16_t)(0x0C00 | ('0' + (scancode & 0xF)));

    if (!(scancode & 0x80)) {
        char ascii = kbd_us[scancode];
        if (ascii != 0) {
            int next = (kb_head + 1) % BUFFER_SIZE;
            if (next != kb_tail) {
                kb_buffer[kb_head] = ascii;
                kb_head = next;
            }
        }
    }
}

char keyboard_getchar(void) {
    if (kb_head == kb_tail) {
        return 0;
    }
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % BUFFER_SIZE;
    return c;
}
