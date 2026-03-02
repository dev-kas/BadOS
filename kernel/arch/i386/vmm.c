#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel/pmm.h>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

extern void load_page_directory(uint32_t*);
extern void enable_paging();
extern void vmm_flush_tlb_entry(uint32_t addr);

void vmm_initialize() {
	// clear the directory (mark everything as not present)
	// bit 1 = read/write, 0 = present. 0x2 means rw but not present
	for (int i = 0; i < 1024; i++) {
		page_directory[i] = 0x00000002;
	}

	// fill the first page table (identity map 0MB to 4MB)
	for (unsigned int i = 0; i < 1024; i++) {
		first_page_table[i] = (i * 4096) | 3;
	}

	// put the page table into the directory
	page_directory[0] = ((unsigned int)first_page_table) | 7;

	load_page_directory(page_directory);
	enable_paging();

	printf("VMM: Paging enabled!\n");
}

void vmm_map_page(uint32_t phys, uint32_t virt, uint32_t flags) {
	uint32_t pd_index = virt >> 22;
	uint32_t pt_index = (virt >> 12) & 0x3FF;
	
	// check if page table exists
	// directory entry format: [ addr 20 bits ] [ flags 12 bits ]
	// bit 0 is present bit
	if (!(page_directory[pd_index] & 0x1)) {
		// page table does not exist
		uint32_t new_pt_phys = pmm_alloc_block();

		uint32_t* new_pt_virt = (uint32_t*)new_pt_phys;
		memset(new_pt_virt, 0, 4096);

		// 0x3 = present (bit 0) | rw (bit 1)
		page_directory[pd_index] = new_pt_phys | 0x7;
	}

	// get page table
	uint32_t pt_phys = page_directory[pd_index] & 0xFFFFF000;
	uint32_t* pt_virt = (uint32_t*)pt_phys;
	pt_virt[pt_index] = (phys & 0xFFFFF000) | (flags & 0xFFF);

	// refresh cache
	vmm_flush_tlb_entry(virt);
}
