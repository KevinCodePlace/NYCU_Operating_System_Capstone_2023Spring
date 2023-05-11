#include <stdint.h>
#include <stddef.h>

#define nop asm volatile("nop")

int utils_string_compare(const char* i, const char* j);
unsigned long utils_atoi(const char *s, int char_size);
void utils_align(void *size, unsigned int s);
uint32_t utils_align_up(uint32_t size, int alignment);
size_t utils_strlen(const char *s);
char *utils_strcpy(char* dst, const char *src);
char *utils_strdup(const char *src);
