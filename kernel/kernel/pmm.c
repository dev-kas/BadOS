#include <kernel/pmm.h>
#include <string.h>
#include <stdio.h>

// 4KB block size
#define BLOCK_SIZE 4096
#define BLOCKS_PER_BYTE 0

static uint32_t* pmm_bitmap = 0;
static uint64_t used_blocks = 0;
static uint64_t max_blocks = 0;

void mmap_set(int bit) {
	pmm_bitmap[bit / 32] |= (1U << (bit % 32));
}

void mmap_unset(int bit) {
	pmm_bitmap[bit / 32] &= ~(1U << (bit % 32));
}

int mmap_test(int bit) {
	return pmm_bitmap[bit / 32] & (1U << (bit % 32));
}

void pmm_initialize(uint64_t mem_size, uint64_t bitmap_start) {
	max_blocks = mem_size / BLOCK_SIZE;
	used_blocks = max_blocks;

	// put the bitmap right where the kernel ends
	pmm_bitmap = (uint64_t*)bitmap_start;

	memset(pmm_bitmap, 0xFF, max_blocks / 8);
	printf("PMM: Bitmap initialized at 0x%x, tracking %d blocks.\n", bitmap_start, max_blocks);
}

void pmm_init_region(uint64_t base, uint64_t size) {
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;

	for (; blocks > 0; blocks--) {
		mmap_unset(align++);
		used_blocks--;
	}

	mmap_set(0);
}

void pmm_deinit_region(uint64_t base, uint64_t size) {
	int align = base / BLOCK_SIZE;
	int blocks = size / BLOCK_SIZE;

	for (; blocks > 0; blocks--) {
		mmap_set(align++);
		used_blocks++;
	}
}

uint64_t pmm_alloc_block() {
	for (uint64_t i = 0; i < max_blocks; i++) {
		if (!mmap_test(i)) {
			mmap_set(i); // mark as read
			used_blocks++;
			return i * BLOCK_SIZE; // return physical addr
		}
	}
	printf("PPM: Out of Memory!\n");
	return 0;
}

void pmm_free_block(uint64_t addr) {
	uint64_t block = addr / BLOCK_SIZE;
	mmap_unset(block);
	used_blocks--;
}
