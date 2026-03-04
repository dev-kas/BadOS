ENTRY(_start)

PHDRS
{
	/* map ELF segments to KEX-compatible groups */
	text PT_LOAD FLAGS(5); /* R | X */
	data PT_LOAD FLAGS(6); /* R | W */
}

SECTIONS
{
	. = 0x1000;

	.text : {
		*(.text*)
	} :text

	.rodata : {
		*(.rodata*)
	} :text

	/* align to page boundary for next segment */
	. = ALIGN(4096);

	.data : {
		*(.data*)
	} :data

	.bss : {
		*(.bss*)
	} :data

	/DISCARD/ : {
		*(.comment)
		*(.note*)
	}
}
