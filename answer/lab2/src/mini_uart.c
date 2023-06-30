#include "peripheral/mini_uart.h"
#include "peripheral/gpio.h"
#include "utils_c.h"

void delay(unsigned int clock)
{
    while (clock--)
    {
        asm volatile("nop");
    }
}

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
    *AUX_MU_IER_REG = 0u;    // Disable receive and transmit interrupts
    *AUX_MU_LCR_REG = 3u;    // Enable 8 bit mode
    *AUX_MU_MCR_REG = 0u;    // Set RTS line to be always high
    *AUX_MU_BAUD_REG = 270u; // Set baud rate to 115200
    *AUX_MU_IIR_REG = 6u;
    *AUX_MU_CNTL_REG = 3u; // Finally, enable transmitter and receiver
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

void uart_send_int(int num, int newline)
{
    char str[256];
    utils_int2str_dec(num, str);
    uart_send_string(str);
    if (newline)
    {
        uart_send_string("\n");
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