#ifndef VGA_H
#define VGA_H
#include <stdint.h>
void vga_init(void);
void vga_clear(void);
void vga_puts(const char* s);
void vga_puthex(uint32_t n);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_set_cursor(int row, int col);
extern int cur_row, cur_col;
#endif
