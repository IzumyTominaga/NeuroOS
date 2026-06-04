#ifndef PMM_H
#define PMM_H
#include <stdint.h>

#define PMM_FRAME_SIZE 4096
#define PMM_FRAMES_MAX 32768  /* 32768 * 4KB = 128MB maximo */

void     pmm_init(uint32_t mmap_addr, uint32_t mmap_len);
uint32_t pmm_alloc_frame(void);
void     pmm_free_frame(uint32_t addr);
uint32_t pmm_free_count(void);
uint32_t pmm_total_count(void);
void     pmm_print_stats(void);

#endif
