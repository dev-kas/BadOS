#ifndef _KERNEL_FS_H
#define _KERNEL_FS_H

#include <stdint.h>

void fs_init(uint32_t ramdisk_start);
void fs_cat(char* filename);
void* fs_get_file(char* filename, uint32_t* out_size);

#endif
