#include <stdio.h>
#include <string.h>
#include <kernel/process.h>
#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/kheap.h>
#include <limine.h>

extern volatile uint64_t uptime_ms;
extern volatile struct limine_framebuffer_request fb_request;

static uint32_t* backbuffer = NULL;
static uint64_t  backbuffer_size = 0;

void syscall_handler(struct registers* regs) {
	switch (regs->rax) {
	case 1: // putchar
		serial_putchar((char)regs->rbx);
		break;
	case 2: // exit
		printf("\nVideo Finished! System Halted.\n");
		while(1) { asm volatile("cli; hlt"); }
		break;
	case 3: // get uptime in ms
		regs->rax = uptime_ms;
		break;
	case 4: // draw 1-bit frame
		{
			uint8_t* frame = (uint8_t*)regs->rbx;
			int w = regs->rcx;
			int h = regs->rdx;
			
			if (!fb_request.response || fb_request.response->framebuffer_count == 0) break;
			struct limine_framebuffer *fb = fb_request.response->framebuffers[0];
			uint32_t* vram = (uint32_t*)fb->address;

			if (!backbuffer) {
			backbuffer_size = (fb->pitch * fb->height);
			backbuffer = (uint32_t*)kmalloc(backbuffer_size);
			memset(backbuffer, 0, backbuffer_size); 
			fast_memcpy(vram, backbuffer, backbuffer_size);
			}

			int start_x = (fb->width - w) / 2;
			int start_y = (fb->height - h) / 2;
			uint64_t fb_width = fb->pitch / 4; 
			int bytes_per_row = w / 8;

			for (int y = 0; y < h; y++) {
			int screen_y = start_y + y;
			if (screen_y < 0 || screen_y >= (int)fb->height) continue;

			uint32_t* dest_ptr = backbuffer + (screen_y * fb_width) + start_x;
			uint8_t* src_ptr = frame + (y * bytes_per_row);

			for (int b = 0; b < bytes_per_row; b++) {
				uint8_t bits = src_ptr[b];
				
				dest_ptr[0] = (bits & 0x01) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[1] = (bits & 0x02) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[2] = (bits & 0x04) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[3] = (bits & 0x08) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[4] = (bits & 0x10) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[5] = (bits & 0x20) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[6] = (bits & 0x40) ? 0xFFFFFFFF : 0xFF000000;
				dest_ptr[7] = (bits & 0x80) ? 0xFFFFFFFF : 0xFF000000;

				dest_ptr += 8;
			}
			}

			for (int y = 0; y < h; y++) {
			int screen_y = start_y + y;
			if (screen_y < 0 || screen_y >= (int)fb->height) continue;

			uint64_t row_offset = (screen_y * fb_width) + start_x;
			
			fast_memcpy(vram + row_offset, backbuffer + row_offset, w * 4);
			}
		}
		break;
	case 5: // yielf / sleep
		asm volatile("sti; hlt; cli");
		break;
	default:
		printf("Unknown syscall %d\n", regs->rax);
		break;
	}
}
