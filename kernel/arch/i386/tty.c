#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

#include "vga.h"

#define KEYBOARD_BUFFER_SIZE 256

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
uint8_t write_ptr = 0;
uint8_t read_ptr = 0;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(size_t line) {
	if (line == 0 || line >= VGA_HEIGHT) return;

	for (size_t row = line; row < VGA_HEIGHT; row++) {
		for (size_t col = 0; col < VGA_WIDTH; col++) {
			VGA_MEMORY[(row - 1) * VGA_WIDTH + col] = VGA_MEMORY[row * VGA_WIDTH + col];
		}
	}
}

void terminal_delete_last_line() {
	uint16_t* ptr;

	ptr = (uint16_t*)0xB8000 + VGA_WIDTH * (VGA_HEIGHT - 1);

	for (size_t x = 0; x < VGA_WIDTH; x++) {
	    ptr[x] = 0;
	}
}

void terminal_putchar(char c) {
	size_t line;
	unsigned char uc = c;

	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			for (line = 1; line <= VGA_HEIGHT - 1; line++)
				terminal_scroll(line);
			terminal_delete_last_line();
			terminal_row = VGA_HEIGHT - 1;
		}
		return;
	}

	if (c == '\b') {
		if (terminal_column > 0) {
		terminal_column--;
		} else if (terminal_row > 0) {
		terminal_row--;
		terminal_column = VGA_WIDTH - 1;
		}
		// if at (0,0) then do nothing
		return;
	}
	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			for (line = 1; line <= VGA_HEIGHT - 1; line++)
				terminal_scroll(line);
			terminal_delete_last_line();
			terminal_row = VGA_HEIGHT - 1;
		}
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

void keyboard_push(char c) {
	uint8_t next_write = (write_ptr + 1) % KEYBOARD_BUFFER_SIZE;
	
	// if buffer is full, drop the key
	if (next_write == read_ptr) {
		return;
	}

	keyboard_buffer[write_ptr] = c;
	write_ptr = next_write;
}

char keyboard_getchar() {
	// wait until there is data (blocking io)
	while (read_ptr == write_ptr) {
		asm volatile("hlt"); // wait for interrupt
	}

	char c = keyboard_buffer[read_ptr];
	read_ptr = (read_ptr + 1) % KEYBOARD_BUFFER_SIZE;
	return c;
}
