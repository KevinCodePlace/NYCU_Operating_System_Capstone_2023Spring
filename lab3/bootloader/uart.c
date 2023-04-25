#include "header/uart.h"

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;
    // P.104 Since I need UART 1 Transmit/Receive Data -> TXD1/RXD1
    // p.102 I find These two in GPIO 14/15 Fun5
    // Since each GPFSEL controls 10 pin, GPFSEL1 controls 10-19
    // That's why I need GPFSEL1  
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15 clear to 0
    r|=(2<<12)|(2<<15);    // set gpio14 and 15 to 010/010 which is alt5
    *GPFSEL1 = r;          // from here activate Trasmitter&Receiver

    //Since We've set alt5, we want to disable basic input/output
    //To achieve this, we need diable pull-up and pull-dwon
    *GPPUD = 0;   //  P101 top. 00- = off - disable pull-up/down 
    //Wait 150 cycles
    //this provides the required set-up time for the control signal 
    r=150; while(r--) { asm volatile("nop"); }
    // GPIO control 54 pins
    // GPPUDCLK0 controls 0-31 pins
    // GPPUDCLK1 controls 32-53 pins
    // set 14,15 bits = 1 which means we will modify these two bits
    // trigger: set pins to 1 and wait for one clock
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup


    r=1000; while(r--) { asm volatile("nop"); }

    /* initialize UART */
    *AUX_ENABLE |=1;       
    //P.9: If set the mini UART is enabled. The UART will
    //immediately start receiving data, especially if the
    //UART1_RX line is low.
    //If clear the mini UART is disabled. That also disables
    //any mini UART register access 
    *AUX_MU_CNTL = 0;
   //P.17 If this bit is set the mini UART receiver is enabled.
   //If this bit is clear the mini UART receiver is disabled
   //Prevent data exchange in initialization process
    *AUX_MU_IER = 0;
   //Set AUX_MU_IER_REG to 0. 
   //Disable interrupt because currently you don’t need interrupt.
    *AUX_MU_LCR = 3;       
   //P.14: 00 : the UART works in 7-bit mode
   //11(3) : the UART works in 8-bit mode
   //Cause 8 bits can use in ASCII, Unicode, Char
    *AUX_MU_MCR = 0;
   //Don’t need auto flow control.
   //AUX_MU_MCR is for basic serial communication. Don't be too smart
    *AUX_MU_BAUD = 270;
   //set BAUD rate to 115200(transmit speed)
   //so we need set AUX_MU_BAUD to 270 to meet the goal
    *AUX_MU_IIR = 0xc6;
   // bit 6 bit 7 No FIFO. Sacrifice reliability(buffer) to get low latency    // 0xc6 = 11000110
   // Writing with bit 1 set will clear the receive FIFO
   // Writing with bit 2 set will clear the transmit FIFO
   // Both bits always read as 1 as the FIFOs are always enabled  
    /* map UART1 to GPIO pins */
    *AUX_MU_CNTL = 3; // enable Transmitter,Receiver
    
}    
    

/**
 * Send a character
 */
void uart_send_char(unsigned int c) {
    /* wait until we can send */
    // P.15 AUX_MU_LSR register shows the data(line) status
    // AUX_MU_LSR bit 5 => 0x20 = 00100000
    // bit 5 is set if the transmit FIFO can accept at least one byte.  
    // &0x20 can preserve 5th bit, if bit 5 set 1 can get !true = false leave loop
    // else FIFO can not accept at lease one byte then still wait 
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    //P.11 The AUX_MU_IO_REG register is primary used to write data to and read data from the
    //UART FIFOs.
    //communicate with(send to) the minicom and print to the screen 
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_get_char() {
    char r;
    /* wait until something is in the buffer */
    //bit 0 is set if the receive FIFO holds at least 1 symbol.
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carriage return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Display a string
 */
void uart_send_string(char* s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_send_char('\r');
        uart_send_char(*s++);
    }
}

/**
 * Display a binary value in hexadecimal
 */
void uart_binary_to_hex(unsigned int d) {
    unsigned int n;
    int c;
    uart_send_string("0x");
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send_char(n);
    }
}


