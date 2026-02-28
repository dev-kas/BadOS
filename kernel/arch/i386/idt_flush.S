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

