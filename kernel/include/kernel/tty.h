#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

void terminal_initialize(void); // initialize all thing in the framebuffer with 0
void terminal_putchar(char c); // but a character and inc cursor
void terminal_write(const char* data, size_t size); // write string by looping with putchar
void terminal_writestring(const char* data); // abstract for write, get len automatically

#endif
