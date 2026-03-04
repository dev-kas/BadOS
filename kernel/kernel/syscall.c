#include <stdio.h>
#include <kernel/process.h>
#include <kernel/tty.h>
#include <kernel/serial.h>
#include <limine.h>

extern volatile uint64_t uptime_ms;
extern volatile struct limine_framebuffer_request fb_request;

void syscall_handler(struct registers* regs) {
	switch (regs->rax) {
	case 1: // putchar
		char c = (char)regs->rbx;
		terminal_putchar(c);
		break;
	case 2: // exit / halt
		printf("\nUser Process Exited.\n");
		while(1) { asm volatile("cli; hlt"); }
		break;
	case 3: // get uptime ms
		regs->rax = uptime_ms;
		break;
	case 4: // draw 1 bit frame
		{
			uint8_t* frame = (uint8_t*)regs->rbx;
			int w = regs->rcx;
			int h = regs->rdx;
			
			if (!fb_request.response || fb_request.response->framebuffer_count == 0) break;
			struct limine_framebuffer *fb = fb_request.response->framebuffers[0];
			uint32_t* screen = (uint32_t*)fb->address;
			
			// center the video on the screen
			int start_x = (fb->width - w) / 2;
			int start_y = (fb->height - h) / 2;
			
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					int i = y * w + x;
					int is_white = (frame[i / 8] & (1 << (i % 8))) != 0;
					uint32_t color = is_white ? 0xFFFFFFFF : 0x00000000;
					
					int screen_x = start_x + x;
					int screen_y = start_y + y;
					
					if (screen_x >= 0 && screen_x < (int)fb->width && screen_y >= 0 && screen_y < (int)fb->height) {
						screen[screen_y * (fb->pitch / 4) + screen_x] = color;
					}
				}
			}
		}
		break;
	default:
		printf("Unknown syscall %d (0x%x)\n", regs->rax, regs->rax);
		break;
	}
}
