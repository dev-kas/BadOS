#include <kernel/process.h>
#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <arch/x86_64/io.h>
#include <stdio.h>

process_t* current_process = NULL;
process_t* process_list = NULL;
volatile uint64_t uptime_ms = 0;

int next_pid = 1;

void multitasking_initialize() {
	current_process = (process_t*)kmalloc(sizeof(process_t));
	current_process->pid = 0;
	current_process->rsp = 0; 
	current_process->page_directory = get_active_page_dir();
	current_process->next = current_process; 
	
	process_list = current_process;
	printf("Multithreading: Initialized\n");
}

void create_kernel_thread(void (*function)(void)) {
	process_t* new_task = (process_t*)kmalloc(sizeof(process_t));
	new_task->pid = next_pid++;
	new_task->page_directory = current_process->page_directory;
	new_task->next = process_list->next;
	process_list->next = new_task;

	uint64_t stack_phys = pmm_alloc_block();
	uint64_t* stack_top = (uint64_t*)(stack_phys + 4096);
	stack_top = (uint64_t*)((uint64_t)stack_top & ~0xF);

	*(--stack_top) = 0x10; // SS (kernel data segment)
	*(--stack_top) = stack_phys + 4096; // RSP (initial stack pointer)
	*(--stack_top) = 0x202; // RFLAGS (interrupts enabled)
	*(--stack_top) = 0x08; // CS (kernel code segment)
	*(--stack_top) = (uint64_t)function; // RIP (instruction pointer)

	*(--stack_top) = 0; // r15
	*(--stack_top) = 0; // r14
	*(--stack_top) = 0; // r13
	*(--stack_top) = 0; // r12
	*(--stack_top) = 0; // r11
	*(--stack_top) = 0; // r10
	*(--stack_top) = 0; // r9
	*(--stack_top) = 0; // r8
	*(--stack_top) = 0; // rax
	*(--stack_top) = 0; // rcx
	*(--stack_top) = 0; // rdx
	*(--stack_top) = 0; // rbx
	*(--stack_top) = 0; // rbp
	*(--stack_top) = 0; // rsi
	*(--stack_top) = 0; // rdi

	new_task->rsp = (uint64_t)stack_top;

	printf("Created task %d\n", new_task->pid);
}

// called by the timer interrupt handler (IRQ0)
uint64_t schedule(uint64_t current_rsp) {
	if (!current_process) return current_rsp;

	current_process->rsp = current_rsp; 
	current_process = current_process->next; 
	return current_process->rsp; 
}

uint64_t irq0_handler(uint64_t rsp) {
	uptime_ms++; // 1 tick = 1ms
	outb(0x20, 0x20); // acknowledge interrupt to PIC
	return schedule(rsp);
}
