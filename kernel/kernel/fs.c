#include <kernel/fs.h>
#include <stdio.h>
#include <string.h>

// standard USTAR header format
struct tar_header {
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12]; // NOTE: size is in octal ascii
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
};

static uint32_t fs_start_addr = 0;

// helper to convert oct string to int
unsigned int get_size(const char* in) {
	unsigned int size = 0;
	unsigned int j = 0;
	unsigned int count = 1;
	for (j = 11; j > 0; j--, count *= 8) size += ((in[j - 1] - '0') * count);
	return size;
}

void fs_init(uint32_t ramdisk_addr) {
	fs_start_addr = ramdisk_addr;
	printf("FileSystem: initialized at 0x%x\n", fs_start_addr);

	struct tar_header* header = (struct tar_header*)fs_start_addr;

	while (header->filename[0] != 0) {
		unsigned int size = get_size(header->size);
		printf("  Found file: %s (%d bytes)\n", header->filename, size);

		// move to next header
		uint32_t next_offset = 512 + (((size + 511) / 512) * 512);
		header = (struct tar_header*)((uint32_t)header + next_offset);
	}
}

void fs_cat (char* target_filename) {
	struct tar_header* header = (struct tar_header*)fs_start_addr;
	
	while (header->filename[0] != 0) {
		unsigned int size = get_size(header->size);

		if (strcmp(header->filename, target_filename) == 0) {
			printf("--- contents of file %s ---\n", target_filename);

			char* content = (char*)((uint32_t)header + 512);
			
			for (unsigned int i = 0; i < size; i++) {
				printf("%c", content[i]);
			}

			printf("--- end of file ---\n");
			return;
		}

		// move to next header
		uint32_t next_offset = 512 + (((size + 511) / 512) * 512);
		header = (struct tar_header*)((uint32_t)header + next_offset);
	}
	printf("File not found: %s\n", target_filename);
}
