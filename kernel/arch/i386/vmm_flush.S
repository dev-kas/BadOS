.global vmm_flush_tlb_entry
vmm_flush_tlb_entry:
	invlpg 4(%esp)
	ret

