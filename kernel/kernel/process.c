#include <kernel/process.h>
#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <arch/i386/io.h>
#include <stdio.h>

// current running process
process_t* current_process = NULL;
// list of all processes
process_t* process_list = NULL;

int next_pid = 1;

void multitasking_initialize() {
	current_process = (process_t*)kmalloc(sizeof(process_t));
	current_process->pid = 0;
	current_process->esp = 0; // will be set by the switcher
	current_process->page_directory = (uint32_t*)page_directory;
	current_process->next = current_process; // loop back to self
	
	process_list = current_process;
	printf("Multithreading: Initialized\n");
}

void create_kernel_thread(void (*function)(void)) {
	process_t* new_task = (process_t*)kmalloc(sizeof(process_t));
	new_task->pid = next_pid++;
	new_task->page_directory = current_process->page_directory;
	new_task->next = process_list->next;
	process_list->next = new_task;

	// allocate a kernel stack for this thread
	// stack grows down so we need the end of allocated block
	uint32_t stack_phys = pmm_alloc_block();
	// TODO: map this properly with vmm
	uint32_t* stack_top = (uint32_t*)(stack_phys + 4096);

	*(--stack_top) = 0x202; // eflagS (interrupts enabled = 1)
	*(--stack_top) = 0x08; // cs (kernel code segment)
	*(--stack_top) = (uint32_t)function; // eip

	*(--stack_top) = 0; // eax
	*(--stack_top) = 0; // ecx
	*(--stack_top) = 0; // edx
	*(--stack_top) = 0; // ebx
	*(--stack_top) = 0; // esp (ignored by popa)
	*(--stack_top) = 0; // ebp
	*(--stack_top) = 0; // esi
	*(--stack_top) = 0; // edi

	new_task->esp = (uint32_t)stack_top;

	printf("Created task %d\n", new_task->pid);
}

// called by the timer interrupt handler (IRQ0)
uint32_t schedule(uint32_t current_esp) {
	if (!current_process) return current_esp;

	current_process->esp = current_esp; // save esp of the task that was interrupted
	current_process = current_process->next; // switch to next task
	return current_process->esp; // return new task's stack pointer

}

uint32_t irq0_handler(uint32_t esp) {
	outb(0x20, 0x20);
	return schedule(esp);
}
