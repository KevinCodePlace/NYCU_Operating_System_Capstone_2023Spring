#include "type.h"
#ifndef __system__
#define __system__

/*
SCN : function
0   : int getpid()
1   : size_t uartread(char buf[], size_t size)
2   : size_t uartwrite(const char buf[], size_t size)
3   : int exec(const char *name, char *const argv[])
4   : int fork()
5   : void exit(int status)
6   : int mbox_call(unsigned char ch, unsigned int *mbox)
7   : void kill(int pid)
*/

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(const char* name, char *const argv[]);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);


#endif