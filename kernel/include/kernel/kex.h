#ifndef KEX_H
#define KEX_H

#include <stdint.h>

#define KEX_MAGIC "KEX\0"
#define KEX_CMD_SEGMENT    1
#define KEX_CMD_RELOC      2
#define KEX_CMD_IMPORT_LIB 3
#define KEX_CMD_IMPORT_SYM 4
#define KEX_CMD_TLS        5
#define KEX_CMD_INTEGRITY  6

#define KEX_ARCH_X86_64  1
#define KEX_ARCH_AARCH64 2
#define KEX_ARCH_RISCV64 3

#pragma pack(push, 1)

typedef struct {
	uint8_t magic[4];
	uint8_t version_major;
	uint8_t version_minor;
	uint16_t arch;
	uint32_t command_count;
	uint32_t required_page_size;
	uint64_t entry_rva;
	uint64_t cmd_offset;
	uint64_t cmd_size;
	uint8_t reserved[16];
} kex_header;

typedef struct {
	uint32_t type;
	uint32_t size;
} kex_cmd;

typedef struct {
	kex_cmd head;
	uint32_t segment_index;
	uint32_t flags;
	uint64_t rva;
	uint64_t mem_size;
	uint64_t file_offset;
	uint64_t file_size;
	uint64_t alignment;
	uint8_t reserved[8];
} kex_segment_cmd;

typedef struct {
	kex_cmd head;
	uint32_t segment_index;
	uint32_t reloc_count;
} kex_reloc_block_cmd;

struct kex_reloc_entry {
	uint64_t rva;
	uint32_t type;
	uint32_t symbol_index;
	int64_t addend;
};

typedef struct {
	kex_cmd head;
	uint64_t template_rva;
	uint64_t template_size;
	uint64_t total_size;
	uint64_t alignment;
} kex_tls_cmd;

typedef struct {
	kex_cmd head;
	uint8_t hash[32]; // sha256
} kex_integrity_cmd;

#pragma pack(pop)

void load_kex_and_run(void* file_data);

#endif
