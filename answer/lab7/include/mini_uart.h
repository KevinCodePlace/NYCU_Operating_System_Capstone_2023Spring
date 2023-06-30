#pragma once
void uart_init();
char uart_read();
char uart_read_raw();
void uart_write(unsigned int c);
void uart_printf(char *fmt, ...);
void uart_flush();
void *handle_uart_irq();
void uart_init_buffer();
int uart_pop(unsigned char *c);
int uart_push(unsigned char c);
void vfs_uart_init();