#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>

struct registers {
	uint32_t edi, esi, ebp, ebx, edx, ecx, eax; // pushed by pusha
	uint32_t int_no, err_code; // interrupt number and error code
	uint32_t eip, cs, eflags, useresp, ss; // pushed by processor automatically
};

typedef struct process {
	int pid; // process id
	uint32_t esp; // stack pointer (where the process was paused)
	uint32_t* page_directory; // virtual memory context (CR3)
	struct process* next;
} process_t;

void multitasking_initialize();
void create_kernel_thread(void (*function)(void));
void switch_task(struct registers* regs); // scheduler

#endif
