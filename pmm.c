#include "pmm.h"
#include "vga.h"

typedef struct {
    uint32_t size;
    uint32_t addr_low, addr_high;
    uint32_t len_low,  len_high;
    uint32_t type;
} __attribute__((packed)) mmap_entry_t;

static uint32_t bitmap[PMM_FRAMES_MAX / 32];
static uint32_t total_frames = 0;
static uint32_t free_frames  = 0;

extern uint32_t _kernel_end;

static void bitmap_set(uint32_t frame) {
    bitmap[frame / 32] |= (1 << (frame % 32));
}
static void bitmap_clear(uint32_t frame) {
    bitmap[frame / 32] &= ~(1 << (frame % 32));
}
static int bitmap_test(uint32_t frame) {
    return bitmap[frame / 32] & (1 << (frame % 32));
}

void pmm_init(uint32_t mmap_addr, uint32_t mmap_len) {
    for (int i = 0; i < PMM_FRAMES_MAX / 32; i++)
        bitmap[i] = 0xFFFFFFFF;

    uint32_t kernel_end = (uint32_t)&_kernel_end;

    uint32_t offset = 0;
    while (offset < mmap_len) {
        mmap_entry_t* e = (mmap_entry_t*)(mmap_addr + offset);

        if (e->type == 1 && e->addr_high == 0) {
            uint32_t start = e->addr_low;
            uint32_t len   = e->len_low;
            uint32_t end   = start + len;

            if (start < 0x100000) start = 0x100000;
            if (start < kernel_end) start = kernel_end;

            start = (start + PMM_FRAME_SIZE - 1) & ~(PMM_FRAME_SIZE - 1);

            for (uint32_t addr = start; addr + PMM_FRAME_SIZE <= end;
                 addr += PMM_FRAME_SIZE) {
                uint32_t frame = addr / PMM_FRAME_SIZE;
                if (frame < PMM_FRAMES_MAX) {
                    bitmap_clear(frame);
                    free_frames++;
                    total_frames++;
                }
            }
        }
        offset += e->size + 4;
    }
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < PMM_FRAMES_MAX / 32; i++) {
        if (bitmap[i] == 0xFFFFFFFF) continue;
        for (int bit = 0; bit < 32; bit++) {
            if (!(bitmap[i] & (1 << bit))) {
                uint32_t frame = i * 32 + bit;
                bitmap_set(frame);
                free_frames--;
                return frame * PMM_FRAME_SIZE;
            }
        }
    }
    return 0;
}

void pmm_free_frame(uint32_t addr) {
    uint32_t frame = addr / PMM_FRAME_SIZE;
    if (frame < PMM_FRAMES_MAX && bitmap_test(frame)) {
        bitmap_clear(frame);
        free_frames++;
    }
}

uint32_t pmm_free_count(void)  { return free_frames; }
uint32_t pmm_total_count(void) { return total_frames; }

static void print_num(uint32_t n) {
    char buf[12]; int i = 10; buf[11] = 0;
    do { buf[--i] = '0' + (n % 10); n /= 10; } while (n && i > 0);
    vga_puts(buf + i);
}

void pmm_print_stats(void) {
    vga_set_color(0x0B, 0x00);
    vga_puts("PMM: total=");
    print_num(total_frames);
    vga_puts(" frames (");
    print_num(total_frames * 4);
    vga_puts(" KB)  libre=");
    print_num(free_frames);
    vga_puts(" frames (");
    print_num(free_frames * 4);
    vga_puts(" KB)\n");
}
