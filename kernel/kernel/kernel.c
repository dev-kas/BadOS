#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/multiboot.h>
#include <kernel/pmm.h>

extern uint32_t _kernel_end;

void kernel_main(uint32_t magic, multiboot_info_t* mboot_ptr) {
	gdt_initialize();
	idt_initialize();
	terminal_initialize();
	serial_initialize();
	pic_remap();

	printf("Boot Magic: 0x%x\n", magic);
	if (magic != 0x2badb002) {
		printf("Error: Invalid Multiboot Magic Number.\n");
		return;
	}

	// check bit 6 to see if we have a valid mmap
	if (!(mboot_ptr->flags & MULTIBOOT_FLAG_MMAP)) {
		printf("Error: No memory map provided by GRUB!\n");
		return;
	}

	// calculate total RAM size from multiboot
	uint32_t mem_size = (mboot_ptr->mem_upper + 1024) * 1024;

	pmm_initialize(mem_size, (uint32_t)&_kernel_end);

	// parse multiboot to unlock valid RAM
	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mboot_ptr->mmap_addr;
	while ((uint32_t)mmap < mboot_ptr->mmap_addr + mboot_ptr->mmap_length) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			pmm_init_region(mmap->addr_low, mmap->len_low);
		}
		mmap = (multiboot_memory_map_t*) ((unsigned int)mmap + mmap->size + sizeof(unsigned int));
	}

	// lock the kernel
	uint32_t bitmap_size = (mem_size / 4096) / 8;
	pmm_deinit_region(0x100000, ((uint32_t)&_kernel_end - 0x100000) + bitmap_size);

	printf("PMM initialized.\n");


	uint32_t a = pmm_alloc_block();
	printf("Allocated page at: 0x%x\n", a);
	uint32_t b = pmm_alloc_block();
	printf("Allocated page at: 0x%x\n", b);

	pmm_free_block(a);
	uint32_t c = pmm_alloc_block();
	printf("Allocated page at: 0x%x (must be same as first one)\n", c);

	asm volatile("sti");

	printf("Hello, Meat world!\n");

	while (1) { asm volatile("hlt"); }
}
