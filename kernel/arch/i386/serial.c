#include <stddef.h>
#include <kernel/serial.h>

#include <arch/i386/io.h>

#define PORT 0x3F8

int serial_initialize(void) {
	outb(PORT + 1, 0x00); // disable all interrupts
	outb(PORT + 3, 0x80); // enable dlab (set baud rate divisor)
	outb(PORT + 0, 0x03); // set divisor to 3 (low byte) 38400 baud
	outb(PORT + 1, 0x00); //                  (high byte)
	outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(PORT + 2, 0xc7); // enable fifo, clear them with 14-byte threshold
	outb(PORT + 4, 0x0b); // irq enabled
	outb(PORT + 4, 0x1e); // set in loopback mode, test the serial chip
	outb(PORT + 0, 0xae); // test value for serial chip
	
	// check if the serial is faulty (ie. not same byte as sent)
	if (inb(PORT + 0) != 0xae) {
		return 1;
	}

	// if model is not faulty set it in normal operation mode
	// (not-loopback with irq enabled and out#1 and out#2 bits enabled)
	outb(PORT + 4, 0x0F);
	return 0;
}

int is_transmit_empty() {
	return inb(PORT + 5) & 0x20;
}

void serial_putchar(char c) {
	while (is_transmit_empty() == 0);
	outb(PORT, c);
}

void serial_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		serial_putchar(data[i]);
}
