.global idt_flush
idt_flush:
	movl 4(%esp), %eax # get pointer passed from c
	lidt (%eax) # load the idt
	ret

.global isr0
.extern isr0_handler
isr0:
	cli # disable other interrupts so we arent interrupted while crashing
	pusha # save all cpu registers
	call isr0_handler
	popa # restore cpu registers
	sti # enable interrupts
	iret # return from this interrupt

.global isr13
.extern isr13_handler
isr13:
	cli
	pusha
	call isr13_handler
	popa
	iret

.global isr14
.extern isr14_handler
isr14:
	cli
	pushl $14
	pusha
	call isr14_handler
	popa
	add $8, %esp # clean up error code and int number
	sti
	iret

.global isr32
.extern irq0_handler # c scheduler
isr32:
	cli
	pusha
	mov %esp, %eax
	push %eax
	call irq0_handler
	mov %eax, %esp
	popa
	sti
	iret

.global isr33
.extern isr33_handler
isr33:
	cli
	pusha
	call isr33_handler
	popa
	sti
	iret

.global isr128
.extern syscall_handler
isr128:
	cli
	pushl $0
	pushl $128
	pusha
	push %esp
	call syscall_handler
	add $4, %esp
	popa
	add $8, %esp
	sti
	iret

