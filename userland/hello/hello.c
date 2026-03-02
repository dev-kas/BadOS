static inline void syscall(int num, int p1, int p2, int p3) {
	asm volatile(
		"int $0x80"
		:
		: "a"(num), "b"(p1), "c"(p2), "d"(p3)
		: "memory"
	);
}

void print(char* str) {
	syscall(1, (int)str, 0, 0);
}

void exit() {
	syscall(2, 0, 0, 0);
}

void _start() {
	print("Hello from a compiled KEX executable!\n");
	print("if you see this, the os loaded a real program");
	exit();
}
