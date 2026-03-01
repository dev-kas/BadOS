#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/multiboot.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/kheap.h>
#include <kernel/fs.h>
#include <kernel/process.h>

#include <string.h>

extern uint32_t _kernel_end;
extern void switch_to_user_mode();

static inline void syscall(int num, int p1, int p2, int p3) {
	// int ret;
	asm volatile(
		"int $0x80"
		: // "=a"(ret) // output: put return value into ret
		: "a"(num), "b"(p1), "c"(p2), "d"(p3) // inputs: eax, ebx, ecx, edx
		: "memory"
	);
	// return ret;
}

void user_print(char* str) {
	syscall(1, (int)str, 0, 0); // eax = 1, ebx = string
}

void user_exit() {
	syscall(2, 0, 0, 0); // eax = 2
}

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

	// check if grub loaded the module
	if (mboot_ptr->mods_count > 0) {
		uint32_t mod_addr = mboot_ptr->mods_addr;
		multiboot_module_t* module = (multiboot_module_t*)mod_addr;

		uint32_t start = module->mod_start;
		uint32_t end = module->mod_end;
		uint32_t len = end - start;

		printf("Module found at 0x%x (size = %d bytes)\n", start, len);

		fs_init(start);
		fs_cat("message.txt");
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

	vmm_initialize();

	// for 1MB heap (256 pages)
	uint32_t heap_start = 0xD0000000;
	uint32_t heap_end = heap_start + (1024 * 1024);

	for (uint32_t i = heap_start; i < heap_end; i += 4096) {
		uint32_t phys = pmm_alloc_block();
		vmm_map_page(phys, i);
	}

	kheap_initialize((void*)heap_start, 256 * 4096);
	multitasking_initialize();

	asm volatile("sti");

	printf("Kernel: Entering User Mode...\n");
	switch_to_user_mode();
	printf("In usermode now. attempting user_print\n");

	user_print("Syscall: Hello from ring 3 via int 0x80!\n");
	printf("attempting another user_print\n");
	user_print("Syscall: this is completely safe...\n");
	printf("attempting user_exit\n");
	user_exit();
	printf("attempting another user_print\n");
	user_print("Syscall: you will never see this\n");
}
