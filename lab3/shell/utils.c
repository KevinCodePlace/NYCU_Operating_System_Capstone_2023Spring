#include "header/utils.h"


int utils_string_compare(const char* str1,const char* str2) {
  for(;*str1 !='\0'||*str2 !='\0';str1++,str2++){
    if(*str1 != *str2) return 0;
    else if(*str1 == '\0' && *str2 =='\0') return 1;
  }
  return 1;
}

unsigned long utils_atoi(const char *s, int char_size) {
    unsigned long num = 0;
    for (int i = 0; i < char_size; i++) {
        num = num * 16;
        if (*s >= '0' && *s <= '9') {
            num += (*s - '0');
        } else if (*s >= 'A' && *s <= 'F') {
            num += (*s - 'A' + 10);
        } else if (*s >= 'a' && *s <= 'f') {
            num += (*s - 'a' + 10);
        }
        s++;
    }
    return num;
}

void utils_align(void *size, unsigned int s) {
	unsigned long* x = (unsigned long*) size;
	unsigned long mask = s-1;
	*x = ((*x) + mask) & (~mask);
}

uint32_t utils_align_up(uint32_t size, int alignment) {
  return (size + alignment - 1) & ~(alignment-1);
}

size_t utils_strlen(const char *s) {
    size_t i = 0;
	while (s[i]) i++;
	return i+1;
}

