#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	// TODO: add proper kernel panic
	printf("kernel: panic: abort()\n");
	asm volatile("cli; hlt");
#else
	printf("Userland Program Aborted.");
	asm volatile(
		"int $0x80"
		:
		: "a"(2), "b"(0), "c"(0), "d"(0)
		: "memory"
	);
#endif
	while (1);
	__builtin_unreachable();
}
