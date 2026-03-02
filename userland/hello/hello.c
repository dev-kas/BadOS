#include <stdio.h>
#include <stdlib.h>

void _start() {
	printf("Hello from a compiled KEX executable!\n");
	printf("if you see this, the os loaded a real program\n");
	abort();
}
