#include "header/utils.h"
#include "header/allocator.h"

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

char *utils_ret_align(char *addr, unsigned alignment)
{
    return (char *) (((unsigned long long) addr + alignment - 1) & ~((unsigned long long) alignment - 1));
}

uint32_t utils_align_up(uint32_t size, int alignment) {
  return (size + alignment - 1) & ~(alignment-1);
}

//with null-terminator -> "hello" return 6
size_t utils_strlen(const char *s) {
    size_t i = 0;
	while (s[i]) i++;
	return i+1;
}


char *utils_strcpy(char* dst, const char *src) {
	char *save = dst;
	while((*dst++ = *src++));
	return save;
}


char *utils_strdup(const char *src) {
	size_t len = utils_strlen(src);
	char *dst = simple_malloc(len);
    if (dst == NULL) { // Check if the memory has been successfully allocated
        return NULL;
    }
    utils_strcpy(dst, src); // Copy the string
    return dst;
}


