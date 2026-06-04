#ifndef VMM_H
#define VMM_H
#include <stdint.h>

#define PAGE_SIZE        4096
#define VMM_FLAG_PRESENT 0x1
#define VMM_FLAG_WRITE   0x2
#define VMM_FLAG_USER    0x4

void      vmm_init(void);
void      vmm_map(uint32_t* dir, uint32_t virt, uint32_t phys, uint32_t flags);
uint32_t* vmm_create_directory(void);
void      vmm_clone_kernel(uint32_t* new_dir);
void      vmm_switch(uint32_t* dir);
uint32_t* vmm_get_kernel_dir(void);
void      vmm_print_dir(uint32_t* dir);
#endif
