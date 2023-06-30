#ifndef _MINI_UART_H
#define _MINI_UART_H


void delay(unsigned int clock);
void enable_uart_interrupt();
void disable_uart_interrupt();
void uart_init();
void uart_send_string(const char* str);
void uart_send_int(int num, int newline);
void uart_send_uint(unsigned int num,int newline);
void uart_send(const char c);
char uart_recv();
char uart_recv_raw();
void uart_hex(unsigned int d);
void uart_dec(unsigned int num);
void uart_handler(void *arg);
void test_uart_async();
char uart_async_recv();
void uart_async_send_string(char *str);
unsigned int uart_printf(char* fmt, ...);
typedef unsigned int Reg;
#endif