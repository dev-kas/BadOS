#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdio.h>
#include <string.h>
#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/kheap.h>
#include <kernel/fs.h>
#include <kernel/process.h>
#include <kernel/kex.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };

__attribute__((used, section(".requests")))
static volatile struct limine_module_request module_request = { .id = LIMINE_MODULE_REQUEST, .revision = 0 };

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };

__attribute__((used, section(".requests")))
volatile struct limine_framebuffer_request fb_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };

uint64_t hhdm_offset = 0;

extern uint64_t _kernel_end;
extern void pit_initialize(uint32_t frequency);

__attribute__((section(".text")))
void _start(void) {
	asm volatile ("cli");
	if (LIMINE_BASE_REVISION_SUPPORTED == false) for (;;) asm("hlt");

	uint64_t cr0, cr4;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 &= ~(1 << 2); // clear EM (emulation)
	cr0 |= (1 << 1);  // set MP (monitor coprocessor)
	asm volatile("mov %0, %%cr0" :: "r"(cr0));
	
	asm volatile("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= (1 << 9) | (1 << 10); // set OSFXSR and OSXMMEXCPT
	asm volatile("mov %0, %%cr4" :: "r"(cr4));

	if (hhdm_request.response) {
		hhdm_offset = hhdm_request.response->offset;
	}

	gdt_initialize();
	idt_initialize();
	terminal_initialize();
	serial_initialize();
	pic_remap();

	printf("Booted in x86_64 using Limine (finally)!\n");

	// PMM config via limine memory map
	uint64_t mem_size = 0;
	uint64_t bitmap_phys = 0;

	if (memmap_request.response) {
		for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
			struct limine_memmap_entry *entry = memmap_request.response->entries[i];
			if (entry->type == LIMINE_MEMMAP_USABLE) {
				if (entry->base + entry->length > mem_size) mem_size = entry->base + entry->length;
			}

			if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= 0x100000 && bitmap_phys == 0) {
				bitmap_phys = entry->base;
			}
		}
	}

	pmm_initialize(mem_size, hhdm_offset + bitmap_phys);

	if (memmap_request.response) {
		for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
			struct limine_memmap_entry *entry = memmap_request.response->entries[i];
			if (entry->type == LIMINE_MEMMAP_USABLE) pmm_init_region(entry->base, entry->length);
		}
	}

	uint64_t bitmap_size = (mem_size / 4096) / 8;
	pmm_deinit_region(bitmap_phys, bitmap_size);

	vmm_initialize();

	uint64_t heap_start = 0xFFFF900000000000ULL;
	for (uint64_t i = heap_start; i < heap_start + (64 * 1024 * 1024); i += 4096) {
		vmm_map_page(pmm_alloc_block(), i, 3);
	}

	kheap_initialize((void*)heap_start, 64 * 1024 * 1024);
	pit_initialize(100); // 100Hz (10ms)
	multitasking_initialize();

	if (module_request.response && module_request.response->module_count > 0) {
		fs_init((uint64_t)module_request.response->modules[0]->address);
		
		uint64_t vid_size;
		void* vid_ptr = fs_get_file("video.bad", &vid_size);
		
		if (vid_ptr) {
			uint64_t pages_needed = (vid_size + 4095) / 4096;
			
			printf("Loading video: %d bytes (%d pages)...\n", vid_size, pages_needed);

			for (uint64_t i = 0; i < pages_needed; i++) {
				uint64_t phys_page = pmm_alloc_block();
				uint8_t* kernel_ptr = (uint8_t*)(phys_page + hhdm_offset);
				uint64_t copy_size = 4096;
				if (i == pages_needed - 1) {
					copy_size = vid_size % 4096;
					if (copy_size == 0) copy_size = 4096;
				}
				
				uint8_t* src_ptr = (uint8_t*)vid_ptr + (i * 4096);
				memcpy(kernel_ptr, src_ptr, copy_size);
				
				vmm_map_page(phys_page, 0x80000000 + (i * 4096), 0x7);
			}
			
			printf("Video mapped successfully to 0x80000000\n");
		} else {
			printf("WARN: video.bad not found in ramdisk!\n");
		}

		uint64_t file_size;
		void* hello_kex = fs_get_file("init.kex", &file_size);
		
		if (hello_kex) {
			load_kex_and_run(hello_kex);
		} else {
			printf("Could not find init.kex\n");
		}
	}

	for (;;) asm volatile ("cli; hlt");
}
