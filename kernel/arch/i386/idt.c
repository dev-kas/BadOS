#include <stdbool.h>

#include <stdio.h>

#include <kernel/idt.h>
#include <kernel/tty.h>

#include <arch/i386/io.h>

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_flush(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	idt[num].base_low = base & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	idt[num].sel = sel;
	idt[num].always0 = 0;
	// TODO: uncomment the OR below when we get to using user-mode
	//   it sets the interrupt gate's privilege level to 3
	idt[num].flags = flags; // | 0x60;
}

// divide by zero
extern void isr0(void);
void isr0_handler(void) {
	printf("EXCEPTION: DIVIDE BY ZERO\n");
	asm volatile ("cli; hlt");
}

// GPF (general protection fault)
extern void isr13(void);
void isr13_handler(void) {
	printf("\n[SECURITY] General protection fault intercepted.\n");
	printf("[SECURITY] A user process tried to do smth illegal.\n");

	while(1) { asm volatile("cli; hlt"); }
}

// #PF (page fault)
extern void isr14(void);
void isr14_handler(void) {
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

	printf("\n[KERNEL PANIC] PAGE FAULT!\n");
	printf("Tried to access unmapped memory at: 0x%x\n", faulting_address);

	while(1) { asm volatile("cli; hlt"); }
}

// IRQ0 - timer
extern void isr32(void);

// IRQ1 - keyboard
extern void isr33(void);
const char kbd_us[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',       
	0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
	'*',  0,  ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,  '-',  0,   0,   0,  '+',  0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0
};
const char kbd_us_shift[128] = {
	0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',   
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',       
	0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,
};
static bool shift_pressed = false;
void isr33_handler(void) {
    uint8_t scancode = inb(0x60);

    // check for shift key
    if (scancode == 0x2A || scancode == 0x36) {
	shift_pressed = true;
	outb(0x20, 0x20);
	return;
    }

    // check for shift release
    if (scancode == 0xAA || scancode == 0xB6) {
	shift_pressed = false;
	outb(0x20, 0x20);
	return;
    }

    if (!(scancode & 0x80)) { 
        char c;

	if (shift_pressed) {
		c = kbd_us_shift[scancode];
	} else {
		c = kbd_us[scancode];
	}
        
        if (c != 0) {
		keyboard_push(c);
		terminal_putchar(c);
        }
    }

    outb(0x20, 0x20); 
}

// syscall
extern void isr128(void);

void idt_initialize(void) {
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base = (uint32_t)&idt;

	for (int i = 0; i < 256; i++) {
		idt_set_gate(i, 0, 0, 0);
	}

	idt_set_gate(  0, (uint32_t)  isr0, 0x08, 0x8E); // divide by zero
	idt_set_gate( 13, (uint32_t) isr13, 0x08, 0x8E); // GPF (general protection fault)
	idt_set_gate( 14, (uint32_t) isr14, 0x08, 0x8E); // #PF (page fault)
	idt_set_gate( 32, (uint32_t) isr32, 0x08, 0x8E); // IRQ0 - timer
	idt_set_gate( 33, (uint32_t) isr33, 0x08, 0x8E); // IRQ1 - keyboard
	idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEE); // syscall

	idt_flush((uint32_t)&idtp);
}

