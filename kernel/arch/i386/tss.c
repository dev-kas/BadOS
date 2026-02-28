#include <kernel/tss.h>
#include <kernel/gdt.h>
#include <string.h>

tss_entry_t tss_entry;

extern void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

extern void tss_flush();

void tss_install(int gdt_idx, uint32_t kernel_ss, uint32_t kernel_esp) {
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	gdt_set_gate(gdt_idx, base, limit, 0xE9, 0x00);

	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.ss0 = kernel_ss;
	tss_entry.esp0 = kernel_esp;

	tss_entry.cs = 0x0b;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;

	tss_flush(); // load the tss selector
}

void tss_set_stack(uint32_t kernel_ss, uint32_t kernel_esp) {
	tss_entry.ss0 = kernel_ss;
	tss_entry.esp0 = kernel_esp;
}
