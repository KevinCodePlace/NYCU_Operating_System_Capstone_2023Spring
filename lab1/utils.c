#include "header/utils.h"


int string_compare(const char* str1,const char* str2) {
  for(;*str1 !='\n'||*str2 !='\0';str1++,str2++){
    if(*str1 != *str2) return 0;
    else if(*str1 == '\n' && *str2 =='\0') return 1;
  }
  return 1;
}
