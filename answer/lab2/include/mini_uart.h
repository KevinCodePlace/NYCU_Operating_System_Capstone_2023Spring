#ifndef _MINI_UART_H
#define _MINI_UART_H


void delay(unsigned int clock);
void uart_init();
void uart_send_string(const char* str);
void uart_send_int(int num, int newline);
void uart_send_uint(unsigned int num,int newline);
void uart_send(const char c);
char uart_recv();
char uart_recv_raw();
void uart_hex(unsigned int d);
#endif