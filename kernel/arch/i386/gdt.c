#include <stdint.h>

#include <kernel/gdt.h>
#include <kernel/tss.h>

struct gdt_entry gdt[6];
struct gdt_ptr gp;

extern void gdt_flush(uint32_t);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = (limit >> 16) & 0x0F;

	gdt[num].granularity |= gran & 0xF0;
	gdt[num].access = access;
}

void gdt_initialize(void) {
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (uint32_t)&gdt;

	// rule 0: null descriptor (required by intel)
	gdt_set_gate(0, 0, 0, 0, 0);

	// rule 1: kernel code (base: 0, limit: 4GB, executable)
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	// rule 2: kernel data (base: 0, limit: 4GB, readable/writable)
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	// rule 3: user code (base: 0, limit: 4GB, type: 0xFA)
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

	// rule 4: user data (base: 0, limit: 4GB, type: 0xF2)
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	// hand the pointer to the cpu via assembly
	gdt_flush((uint32_t)&gp);
	tss_install(5, 0x10, 0);
}

