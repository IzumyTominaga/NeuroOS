#include "vga.h"
#include <stdint.h>
#define VGA_MEM ((uint16_t*)0xB8000)
#define W 80
#define H 25
int cur_row=0,cur_col=0;
static uint8_t cur_fg=0x0A,cur_bg=0x00;
static uint16_t make_cell(char c){
    return(uint16_t)((cur_bg<<4|cur_fg)<<8)|(uint8_t)c;}
void vga_clear(void){
    for(int i=0;i<W*H;i++)VGA_MEM[i]=make_cell(' ');
    cur_row=cur_col=0;}
void vga_set_color(uint8_t fg,uint8_t bg){cur_fg=fg;cur_bg=bg;}
void vga_set_cursor(int row,int col){cur_row=row;cur_col=col;}
static void scroll(void){
    for(int r=0;r<H-1;r++)
        for(int c=0;c<W;c++)VGA_MEM[r*W+c]=VGA_MEM[(r+1)*W+c];
    for(int c=0;c<W;c++)VGA_MEM[(H-1)*W+c]=make_cell(' ');
    cur_row=H-1;}
static void putchar_at(char c){
    if(c=='\n'){cur_col=0;cur_row++;
        if(cur_row>=H)scroll();return;}
    VGA_MEM[cur_row*W+cur_col]=make_cell(c);
    cur_col++;
    if(cur_col>=W){cur_col=0;cur_row++;}
    if(cur_row>=H)scroll();}
void vga_puts(const char*s){for(int i=0;s[i];i++)putchar_at(s[i]);}
void vga_puthex(uint32_t n){
    const char*h="0123456789ABCDEF";
    char buf[9];buf[8]=0;
    for(int i=7;i>=0;i--){buf[i]=h[n&0xF];n>>=4;}
    vga_puts(buf);}
void vga_init(void){vga_clear();}
