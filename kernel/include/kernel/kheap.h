#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stddef.h>
#include <stdint.h>

void kheap_initialize(void* start_addr, size_t size);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
