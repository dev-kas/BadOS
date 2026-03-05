#include <string.h>
#include <stdint.h>

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;

	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];

	return dstptr;
}

void* fast_memcpy(void* dest, const void* src, size_t count) {
	size_t qwords = count / 8;
	size_t bytes = count % 8;

	asm volatile (
		"rep movsq"
		: "+D"(dest), "+S"(src), "+c"(qwords)
		:
		: "memory"
	);

	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;
	for (size_t i = 0; i < bytes; i++) {
		*d++ = *s++;
	}
	return dest;
}
