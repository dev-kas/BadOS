.global switch_to_user_mode
switch_to_user_mode:
	cli

	# get the entry_point arg from c stack
	mov 4(%esp), %ebx

	# gdt entry 4 is 0x20, so 0x20 | 3 (ring 3) = 0x23
	mov $0x23, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# construct iret frame
	pushl $0x23 # ss (user data)
	pushl $0x800000 # esp (user stack)
	pushf # eflags

	popl %eax
	orl $0x200, %eax
	pushl %eax

	pushl $0x1B # push cs (user code segment)
	pushl %ebx # eip
	iret # jump to user mode

