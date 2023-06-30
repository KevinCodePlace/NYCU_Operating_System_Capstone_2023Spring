#ifndef _MAILBOX_H
#define _MAILBOX_H

#include "peripheral/base.h"

#define MAILBOX_READ        ((volatile unsigned int *)(MAILBOX_BASE))
#define MAILBOX_STATUS      ((volatile unsigned int *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE       ((volatile unsigned int *)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY       0x40000000
#define MAILBOX_FULL        0x80000000
#define MAILBOX_RESPONSE    0x80000000
#define ARM_MEMORY          0x00010005

#define GET_BOARD_REVISION  0x00010002
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000


int mailbox_call();
void get_board_revision();
void get_arm_memory();

#endif