#include "peripheral/mini_uart.h"
#include "peripheral/gpio.h"
#include "sprintf.h"
#include "utils_c.h"
#include <stddef.h>
#include <stdarg.h>
#include "mini_uart.h"

#define ENABLE_RECEIVE_INTERRUPTS_BIT (1 << 0)
#define ENABLE_TRANSMIT_INTERRUPTS_BIT (1 << 1)
#define AUX_INT_BIT (1 << 29)

#define BUFFER_MAX_SIZE 256u

char read_buf[BUFFER_MAX_SIZE];
char write_buf[BUFFER_MAX_SIZE];
int read_buf_start, read_buf_end;
int write_buf_start, write_buf_end;


void enable_uart_interrupt() { *ENB_IRQS1 = AUX_IRQ; }

void disable_uart_interrupt() { *DISABLE_IRQS1 = AUX_IRQ; }

void set_transmit_interrupt() { *AUX_MU_IER_REG |= 0x2; }

void clear_transmit_interrupt() { *AUX_MU_IER_REG &= ~(0x2); }

void uart_init()
{
    unsigned int selector;

    selector = *GPFSEL1;
    selector &= ~(7u << 12); // clean gpio14
    selector |= 2u << 12;    // set alt5 for gpio14
    selector &= ~(7u << 15); // clean gpio15
    selector |= 2u << 15;    // set alt5 for gpio 15
    *GPFSEL1 = selector;

    *GPPUD = 0;  // set the required control signal (i.e. Pull-up or Pull-Down )
    delay(150u); // provides the required set-up time for the control signal
    *GPPUDCLK0 = (1u << 14) | (1u << 15);
    delay(150u);
    *GPPUDCLK0 = 0u;
    *AUX_ENABLE = 1u;        // Enable mini uart (this also enables access to its registers)
    *AUX_MU_CNTL_REG = 0u;   // Disable auto flow control and disable receiver and transmitter (for now)
    *AUX_MU_IER_REG = 1u;    // Enable receive
    *AUX_MU_LCR_REG = 3u;    // Enable 8 bit mode
    *AUX_MU_MCR_REG = 0u;    // Set RTS line to be always high
    *AUX_MU_BAUD_REG = 270u; // Set baud rate to 115200
    *AUX_MU_IIR_REG = 6;

    *AUX_MU_CNTL_REG = 3; // Finally, enable transmitter and receiver

    read_buf_start = read_buf_end = 0;
    write_buf_start = write_buf_end = 0;
    // enable_uart_interrupt();
}

void uart_send(const char c)
{
    /*
    bit_5 == 1 -> writable
    0x20 = 0000 0000 0010 0000
    ref BCM2837-ARM-Peripherals p5
    */
    if (c == '\n')
    {
        uart_send('\r');
    }
    while (!(*(AUX_MU_LSR_REG)&0x20))
    {
    }
    *AUX_MU_IO_REG = c;
}
char uart_recv()
{
    /*
    bit_0 == 1 -> readable
    0x01 = 0000 0000 0000 0001
    ref BCM2837-ARM-Peripherals p5
    */
    while (!(*(AUX_MU_LSR_REG)&0x01))
    {
    }
    char temp = *(AUX_MU_IO_REG)&0xFF;
    return temp == '\r' ? '\n' : temp;
}

char uart_recv_raw()
{
    /*
    bit_0 == 1 -> readable
    0x01 = 0000 0000 0000 0001
    ref BCM2837-ARM-Peripherals p5
    */
    while (!(*(AUX_MU_LSR_REG)&0x01))
    {
    }
    char temp = *(AUX_MU_IO_REG)&0xFF;
    return temp;
}

void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_send(*str++);
    }
}

void uart_send_uint(unsigned int num, int newline)
{
    char str[256];
    utils_uint2str_dec(num, str);
    uart_send_string(str);
    if (newline)
    {
        uart_send_string("\n");
    }
}

void uart_hex(unsigned int d)
{
    unsigned int n;
    int c;
    uart_send_string("0x");
    for (c = 28; c >= 0; c -= 4)
    {
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x57 : 0x30;
        uart_send(n);
    }
}
void uart_dec(unsigned int num)
{
    if (num == 0)
        uart_send('0');
    else
    {
        if (num >= 10)
            uart_dec(num / 10);
        uart_send(num % 10 + '0');
    }
}

unsigned int uart_printf(char *fmt, ...)
{
    char dst[100];
    // __builtin_va_start(args, fmt): "..." is pointed by args
    // __builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    unsigned int ret = vsprintf(dst, fmt, args);
    uart_send_string(dst);
    return ret;
}

/*
    async part
*/

void uart_handler()
{
    disable_uart_interrupt();
    int RX = (*AUX_MU_IIR_REG & 0x4);
    int TX = (*AUX_MU_IIR_REG & 0x2);
    if (RX)
    {
        char c = (char)(*AUX_MU_IO_REG);
        read_buf[read_buf_end++] = c;
        if (read_buf_end == BUFFER_MAX_SIZE)
            read_buf_end = 0;
    }
    else if (TX)
    {
        while (*AUX_MU_LSR_REG & 0x20)
        {
            if (write_buf_start == write_buf_end)
            {
                clear_transmit_interrupt();
                break;
            }
            char c = write_buf[write_buf_start++];
            *AUX_MU_IO_REG = c;
            if (write_buf_start == BUFFER_MAX_SIZE)
                write_buf_start = 0;
        }
    }
    enable_uart_interrupt();
}

char uart_async_recv()
{
    // wait until there are new data
    while (read_buf_start == read_buf_end)
    {
        asm volatile("nop");
    }
    char c = read_buf[read_buf_start++];
    if (read_buf_start == BUFFER_MAX_SIZE)
        read_buf_start = 0;
    return c == '\r' ? '\n' : c;
}

void uart_async_send_string(char *str)
{

    for (int i = 0; str[i]; i++)
    {
        if (str[i] == '\n')
        {
            write_buf[write_buf_end++] = '\r';
            write_buf[write_buf_end++] = '\n';
            continue;
        }
        write_buf[write_buf_end++] = str[i];
        if (write_buf_end == BUFFER_MAX_SIZE)
            write_buf_end = 0;
    }
    set_transmit_interrupt();
}

void uart_async_send(char c)
{
    if (c == '\n')
    {
        write_buf[write_buf_end++] = '\r';
        write_buf[write_buf_end++] = '\n';
        set_transmit_interrupt();
        return;
    }
    write_buf[write_buf_end++] = c;
    if (write_buf_end == BUFFER_MAX_SIZE)
        write_buf_end = 0;
    set_transmit_interrupt();
}

void test_uart_async()
{
    enable_uart_interrupt();
    delay(15000);
    char buffer[BUFFER_MAX_SIZE];
    size_t index = 0;
    while (1)
    {
        buffer[index] = uart_async_recv();
        if (buffer[index] == '\n')
        {
            break;
        }
        index++;
    }
    buffer[index + 1] = '\0';
    uart_async_send_string(buffer);
    disable_uart_interrupt();
}
