#ifndef _MINI_UART_H
#define _MINI_UART_H

void uart_init();
void uart_send_string(char* str);
void uart_send_int(int num, int newline);
void uart_send_uint(unsigned int num,int newline);
void uart_send(char c);
char uart_recv();
void uart_hex(unsigned int d);
#endif