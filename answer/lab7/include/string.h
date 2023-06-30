#pragma once
#define size_t unsigned long
void itoa(int value, char *s);
void ftoa(float *value, int precise, char *s);
void i16toa(int value, char *s, int digits);
int strcmp(char *string1, char *string2);
int strncmp(char *string1, char *string2, int length);
void strcpy(char *string1, char* string2, int length);
unsigned long long letobe(unsigned long long o);
void strccat(char *string1, char* string2, char *string_out);
int atoi(char *s);
int a16toi(char *s);
void *memset(void *str, int c, size_t n);
char * strtok(char * str, const char * delimiters);
