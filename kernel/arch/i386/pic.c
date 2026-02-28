#include <stdint.h>
#include <arch/i386/io.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

void pic_remap(void) {
	// start the init sequence in cascade mode
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();

	// set PIC1 offset to 32 (0x20)
	outb(PIC1_DATA, 0x20);
	io_wait();
	// set PIC2 offset to 40 (0x28)
	outb(PIC2_DATA, 0x28);
	io_wait();

	// tell PIC1 that there is a slave PIC at IRQ2
	outb(PIC1_DATA, 4);
	io_wait();
	// tell PIC2 its cascade identity
	outb(PIC2_DATA, 2);
	io_wait();

	// set 8086/88 (x86) mode
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	// restore saved masks
	outb(PIC1_DATA, 0xFC);
	outb(PIC2_DATA, 0xFF);
}
