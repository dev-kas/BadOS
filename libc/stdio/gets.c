#include <stdio.h>
#include <kernel/tty.h>

void gets(char* buffer, int max_len) {
	int i = 0;
	while (i < max_len - 1) {
		char c = keyboard_getchar();

		if (c == '\n') {
			buffer[i] = '\0';
			terminal_putchar('\n');
			return;
		} else if (c == '\b') {
			if (i > 0) i--;
		} else {
			buffer[i++] = c;
		}
	}
	buffer[i] = '\0';
}
