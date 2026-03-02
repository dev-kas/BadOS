#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <kernel/kex.h>

extern void switch_to_user_mode(uint32_t entry_point);

void load_kex_and_run(void* file_data) {
	kex_header* hdr = (kex_header*)file_data;

	// verify magic number
	if (hdr->magic[0] != 'K' || hdr->magic[1] != 'E' ||
		hdr->magic[2] != 'X' || hdr->magic[3] != '\0') {
		printf("Error: Invalid KEX magic!\n");
		return;
	}

	printf("KEX Loader: Found Valid Executable!\n");
	printf("  Arch: %d, Entry RVA: 0x%x\n", hdr->arch, (uint32_t)hdr->entry_rva);

	// parse commands
	uint32_t current_offset = hdr->cmd_offset;

	for (uint32_t i = 0; i < hdr->command_count; i++) {
		kex_cmd* cmd = (kex_cmd*)((uint32_t)file_data + current_offset);

		if (cmd->type == KEX_CMD_SEGMENT) {
			kex_segment_cmd* seg = (kex_segment_cmd*)cmd;

			printf("  Loading Segment: RVA 0x%x, MemSz %d, FileSz %d\n",
				(uint32_t)seg->rva, (uint32_t)seg->mem_size, (uint32_t)seg->file_size);

			// zero-initialize the memory
			memset((void*)(uint32_t)seg->rva, 0, seg->mem_size);

			// copy the data from the KEX file into the RAM
			if (seg->file_size > 0) {
				memcpy((void*)(uint32_t)seg->rva,
					(void*)((uint32_t)file_data + (uint32_t)seg->file_offset),
					seg->file_size);
			}
		} else if (cmd->type == KEX_CMD_INTEGRITY) {
			// TODO: implement sha256 verification then verify
			printf("  Skipping integrity hash veification\n");
		}

		// move to next cmd based on the size defined in the header
		current_offset += cmd->size;
	}

	printf("Jumping to KEX entry point...\n");
	switch_to_user_mode((uint32_t)hdr->entry_rva);
}
