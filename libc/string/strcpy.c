#include <string.h>

char* strcpy(char* dest, const char* src) {
	char* r = dest;
	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}
	return r;
}

