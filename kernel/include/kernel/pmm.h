#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdint.h>
#include <stddef.h>

void pmm_initialize(uint32_t mem_upper_limit, uint32_t kernel_end);
uint32_t pmm_alloc_block();
void pmm_free_block(uint32_t block_addr);
void pmm_deinit_region(uint32_t base, uint32_t size);
void pmm_init_region(uint32_t base, uint32_t size);

#endif
