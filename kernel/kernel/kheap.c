#include <kernel/kheap.h>
#include <stdio.h>

// block header
struct kheap_block {
	struct kheap_block* next; // next free block
	size_t size; // size of this block including header
	int is_free; // marks this block as free (1 = free, 0 = no)
};

static struct kheap_block* free_list = NULL;

void kheap_initialize(void* start_addr, size_t total_size) {
	free_list = (struct kheap_block*)start_addr;
	free_list->size = total_size;
	free_list->is_free = 1;
	free_list->next = NULL;
}

void* kmalloc(size_t size) { // best-fit allocator
	size_t total_size = size + sizeof(struct kheap_block);

	struct kheap_block* current = free_list;
	while (current) {
		if (current->is_free && current->size >= total_size) {
			// found a block that fits
			// check if there's enough space for another header
			if (current->size > total_size + sizeof(struct kheap_block)) {
				struct kheap_block* new_block = (struct kheap_block*)((char*)current + total_size);
				new_block->size = current->size - total_size;
				new_block->is_free = 1;
				new_block->next = current->next;

				current->size = total_size;
				current->next = new_block;
			}
			current->is_free = 0;
			return (void*)((char*)current + sizeof(struct kheap_block));
		}
		current = current->next;
	}

	printf("KHEAP: OOM (Out of Memory)\n");
	return NULL;
}

void kfree(void* ptr) { // merge
	if (!ptr) return;

	// get the header
	struct kheap_block* block = (struct kheap_block*)((char*)ptr - sizeof(struct kheap_block));
	block->is_free = 1;

	// merge with next block if its free (optimization)
	if (block->next && block->next->is_free) {
		block->size += block->next->size;
		block->next = block->next->next;
	}
}
