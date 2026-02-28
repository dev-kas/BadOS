.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0X1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	# pass multiboot info to kernel_main
	pushl %ebx # ptr to the multiboot information structure
	pushl %eax # magic number (0x2BADB002)

	call _init
	call kernel_main

	cli
1:	hlt
	jmp 1b
.size _start, . - _start

.global load_page_directory
load_page_directory:
	push %ebp
	mov %esp, %ebp
	mov 8(%ebp), %eax
	mov %eax, %cr3 # cr3 holds the address of page directory
	mov %ebp, %esp
	pop %ebp
	ret

.global enable_paging
enable_paging:
	push %ebp
	mov %esp, %ebp
	mov %cr0, %eax
	or $0x80000000, %eax # set the 31st bit (PG bit) to 1
	mov %eax, %cr0
	mov %ebp, %esp
	pop %ebp
	ret

