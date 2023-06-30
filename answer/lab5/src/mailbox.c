#include "peripheral/mailbox.h"
#include "utils_c.h"
#include "mini_uart.h"

unsigned int __attribute__((aligned(16))) mailbox[8];

unsigned int mailbox_call(unsigned char channel, unsigned int *_mailbox)
{
    unsigned int readChannel = (((unsigned int)((unsigned long)_mailbox) & ~0xF) | (channel & 0xF));
    while (*MAILBOX_STATUS & MAILBOX_FULL)
    {
    }
    *MAILBOX_WRITE = readChannel;
    while (1)
    {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY)
        {
        }
        if (readChannel == *MAILBOX_READ)
        {
            return _mailbox[1] == MAILBOX_RESPONSE;
        }
    }
    return 0;
}

unsigned int get_board_revision()
{
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4;                  // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;
    mailbox_call(MAILBOX_CH_PROP, mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.
    // printf("0x%x\n", mailbox[5]); // it should be 0xa020d3 for rpi3 b+
    uart_hex(mailbox[5]);
    uart_send_string("\n");
    return mailbox[5];
}


void get_arm_memory()
{
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = ARM_MEMORY; // tag identifier
    mailbox[3] = 8;          // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;
    mailbox_call(MAILBOX_CH_PROP, mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.
    uart_send_string("Arm base address: ");
    uart_hex(mailbox[5]);
    uart_send_string("\n");
    uart_send_string("Arm memory size: ");
    uart_hex(mailbox[6]);
    uart_send_string("\n");
}
