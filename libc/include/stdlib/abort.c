#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	// TODO: add proper kernel panic
	printf("kernel: panic: abort()\n");
	asm volatile("hlt");
#else
	// TODO: abnormally terminate process as if by SIGABRT
	printf("panic()\n");
#endif
	while (1) { };
	__builtin_unreachable();
}
