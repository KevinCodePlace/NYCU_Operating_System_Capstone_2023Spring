#include "mini_uart.h"
#include "peripheral/mmio.h"
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

#define MBOX_REQUEST              0
#define MBOX_CH_PROP              8
#define MBOX_TAG_LAST             0

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
    uart_printf("ARM memory base addr: 0x%x size: 0x%x\n",mailbox[5],mailbox[6]);

}

unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
unsigned long long lfb;                         /* raw frame buffer address */

int set_vc(unsigned int fb_info[4]){
  unsigned int __attribute__((aligned(16))) mbox[36];

  mbox[0] = 35 * 4;
  mbox[1] = MBOX_REQUEST;

  mbox[2] = 0x48003; // set phy wh
  mbox[3] = 8;
  mbox[4] = 8;
  mbox[5] = fb_info[0]; // FrameBufferInfo.width
  mbox[6] = fb_info[1];  // FrameBufferInfo.height

  mbox[7] = 0x48004; // set virt wh
  mbox[8] = 8;
  mbox[9] = 8;
  mbox[10] = fb_info[0]; // FrameBufferInfo.virtual_width
  mbox[11] = fb_info[1];  // FrameBufferInfo.virtual_height

  mbox[12] = 0x48009; // set virt offset
  mbox[13] = 8;
  mbox[14] = 8;
  mbox[15] = 0; // FrameBufferInfo.x_offset
  mbox[16] = 0; // FrameBufferInfo.y.offset

  mbox[17] = 0x48005; // set depth
  mbox[18] = 4;
  mbox[19] = 4;
  mbox[20] = 32; // FrameBufferInfo.depth

  mbox[21] = 0x48006; // set pixel order
  mbox[22] = 4;
  mbox[23] = 4;
  mbox[24] = fb_info[3]; // RGB, not BGR preferably

  mbox[25] = 0x40001; // get framebuffer, gets alignment on request
  mbox[26] = 8;
  mbox[27] = 8;
  mbox[28] = fb_info[2]; // FrameBufferInfo.pointer
  mbox[29] = 0;    // FrameBufferInfo.size

  mbox[30] = 0x40008; // get pitch
  mbox[31] = 4;
  mbox[32] = 4;
  mbox[33] = 0; // FrameBufferInfo.pitch

  mbox[34] = MBOX_TAG_LAST;

  // this might not return exactly what we asked for, could be
  // the closest supported resolution instead
  mailbox_call(mbox, MBOX_CH_PROP);
  if ( mbox[20] == 32 && mbox[28] != 0) {
    mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
    width = mbox[5];        // get actual physical width
    height = mbox[6];       // get actual physical height
    pitch = mbox[33];       // get number of bytes per line
    isrgb = mbox[24];       // get the actual channel order
    lfb = ((unsigned long long)mbox[28] | 0xFFFF000000000000);
  } else {
    uart_printf("Unable to set screen resolution to %dx%dx32\n",fb_info[0],fb_info[1]);
  }
}

void fb_splash(unsigned char color, unsigned long ptr){
  *((unsigned char *)(lfb + ptr)) = color;
}

void fb_splash2() {
    int x, y;
    unsigned char *ptr = lfb;
    unsigned int white = 255 << 16 | 255 << 8 | 255;  // A B G R
    unsigned int black = 0;
    unsigned int current, start = black, spacing = 40;
    unsigned long long tmp = 0;

    for (y = 0; y < height; y++) {
        if (y % spacing == 0 && y != 0) {
            start = (start == white) ? black : white;
        }
        current = start;
        for (x = 0; x < width; x++) {
            if (x % spacing == 0 && x != 0) {
                current = (current == white) ? black : white;
            }
            fb_splash(current,tmp);
            tmp+=4;
        }
    }
}