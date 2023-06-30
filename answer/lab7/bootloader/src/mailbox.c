#include "mini_uart.h"
#include "mmio.h"
#include "string.h"
typedef unsigned int uint32;
typedef unsigned char byte;

#define MAILBOX_BASE              MMIO_BASE + 0xb880

#define MAILBOX_READ              (uint32*)(MAILBOX_BASE)
#define MAILBOX_STATUS            (uint32*)(MAILBOX_BASE+0x18)
#define MAILBOX_WRITE             (uint32*)(MAILBOX_BASE+0x20)

#define MAILBOX_EMPTY             0x40000000
#define MAILBOX_FULL              0x80000000
#define MAILBOX_CODE_BUF_RES_SUCC 0x80000000
#define MAILBOX_CODE_BUF_REQ      0x00000000
#define MAILBOX_CODE_TAG_REQ      0x00000000

#define GET_ARM_MEMORY            0x00010005
#define GET_BOARD_REVISION        0x00010002
#define REQUEST_CODE              0x00000000
#define REQUEST_SUCCEED           0x80000000
#define REQUEST_FAILED            0x80000001
#define TAG_REQUEST_CODE          0x00000000
#define END_TAG                   0x00000000

void mailbox_call(uint32 *mailbox,byte channel){
  uint32 r = (uint32)(((unsigned long)mailbox) & (~0xF)) | (channel & 0xF);
  while (*MAILBOX_STATUS & MAILBOX_FULL) {
  }
  *MAILBOX_WRITE = r;
  while(1){
    while(*MAILBOX_STATUS & MAILBOX_EMPTY){
    }
    
    if (r == *MAILBOX_READ){
      return mailbox[1] == MAILBOX_CODE_BUF_RES_SUCC;
    }
  }
  return 0;
}

void get_board_revision(){
  uint32 __attribute__((aligned(16))) mailbox[7];
  mailbox[0] = 7 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_BOARD_REVISION; // tag identifier
  mailbox[3] = 4; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  // tags end
  mailbox[6] = END_TAG;

  mailbox_call(mailbox,8); // message passing procedure call, you should implement it following the 6 steps provided above.
  char s[11];
  i16toa(mailbox[5],s,8);
  uart_printf("Board Revision: 0x"); // it should be 0xa020d3 for rpi3 b+
  uart_printf(s);
  uart_printf("\n");
}

void get_arm_memory() {
    unsigned int __attribute__((aligned(16))) mailbox[8];
    mailbox[0] = 8 * 4;  // buffer size in bytes
    mailbox[1] = MAILBOX_CODE_BUF_REQ;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY;          // tag identifier
    mailbox[3] = 8;                       // maximum of request and response value buffer's length.
    mailbox[4] = MAILBOX_CODE_TAG_REQ;    // tag code
    mailbox[5] = 0;                       // base address
    mailbox[6] = 0;                       // size in bytes
    mailbox[7] = 0x0;                     // end tag
    // tags end
    mailbox_call(mailbox, 8);
    uart_printf("ARM memory base addr: 0x");
    char a[11];
    i16toa(mailbox[5],a,8);
    uart_printf(a);
    uart_printf(" size: 0x");
    char b[11];
    i16toa(mailbox[6],b,8);
    uart_printf(b);
    uart_printf("\n");
}
