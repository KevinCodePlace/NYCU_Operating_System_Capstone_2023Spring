#ifndef UART_H
#define UART_H

#include "gpio.h"
#include <stdint.h>
#define UART_BUFFER_SIZE 1024
#define ENDL uart_send_char('\r');uart_send_char('\n');

void uart_init();
void uart_send_char(unsigned int c);
char uart_get_char();
void uart_send_string(char* s);
void uart_hex(unsigned long long d);
void uart_hexll(uint64_t d);
void uart_enable_interrupt();
int uart_async_read(char *buffer);
void uart_async_write(const char *buffer, int length);
void uart_async_send(const char *str);


extern char uart_read_buffer[UART_BUFFER_SIZE];
extern char uart_write_buffer[UART_BUFFER_SIZE];
extern int uart_read_index;
extern int uart_read_head;
extern int uart_write_index;
extern int uart_write_head;

#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

#endif
