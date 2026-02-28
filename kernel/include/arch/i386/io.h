#ifndef _ARCH_I386_IO_H
#define _ARCH_I386_IO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
	asm volatile ( "outb %0, %1": : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ( "inb %1, %0": "=a"(ret) : "Nd"(port) );
	return ret;
}

static inline void io_wait(void) {
	// port 0x80 is used for checkpoints during POST
	// writing to it takes exactly long enough to delay the cpu for the pic
	asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

#endif
