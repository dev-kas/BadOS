#include <stdio.h>
#include <kernel/process.h>
#include <kernel/tty.h>
#include <kernel/serial.h>

void syscall_handler(struct registers* regs) {
	switch (regs->eax) {
	case 1: // putchar
		char c = (char)regs->ebx;
		terminal_putchar(c);
		serial_putchar(c);
		break;
	case 2: // exit / halt
		printf("\nUser Process Exited.\n");
		while(1) { asm volatile("cli; hlt"); }
		break;
	default:
		printf("Unknown syscall\n");
		printf("BEGIN OF REGISTER DUMP\n");
		printf("  EAX: %d (%x)\n", regs->eax, regs->eax);
		printf("  ECX: %d (%x)\n", regs->ecx, regs->ecx);
		printf("  EBX: %d (%x)\n", regs->ebx, regs->ebx);
		printf("  EDX: %d (%x)\n", regs->edx, regs->edx);

		printf("  EBP: %d (%x)\n", regs->ebp, regs->ebp);
		printf("  ESI: %d (%x)\n", regs->esi, regs->esi);
		printf("  EDI: %d (%x)\n", regs->edi, regs->edi);
		printf("  ESP: %d (%x)\n", regs->esp, regs->esp);

		printf("  EIP: %d (%x)\n", regs->eip, regs->eip);
		printf("   CS: %d (%x)\n", regs->cs, regs->cs);
		printf("EFLAGS: %d (%x)\n", regs->eflags, regs->eflags);

		printf("USERESP: %d (%x)\n", regs->useresp, regs->useresp);
		printf("     SS: %d (%x)\n", regs->ss, regs->ss);

		printf("END OF REGISTER DUMP\n");
		break;
	}
}
