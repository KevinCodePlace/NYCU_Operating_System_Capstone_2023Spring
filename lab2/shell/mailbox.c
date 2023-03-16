#include "header/mailbox.h"

volatile unsigned int  __attribute__((aligned(16))) mailbox[36];

int mailbox_call() 
{
    unsigned int r = (((unsigned int)((unsigned long)&mailbox)&~0xF) | (MBOX_CH_PROP&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ){
            /* is it a valid successful response? */
	    if(mailbox[1] == MBOX_RESPONSE)
		    return 1; 
	}
    }
    return 0;
}

void get_board_revision(){
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = MBOX_REQUEST;    
    // tags begin
    mailbox[2] = MBOX_TAG_GETBOARD; // tag identifier
    mailbox[3] = 4;          // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = MBOX_TAG_LAST;
    mailbox_call(); // message passing procedure call, we should implement it following the 6 steps provided above.
}

void get_arm_mem(){
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = MBOX_REQUEST;    
    // tags begin
    mailbox[2] = MBOX_TAG_GETARMMEM; // tag identifier
    mailbox[3] = 8;          // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = MBOX_TAG_LAST;
    mailbox_call(); // message passing procedure call, we should implement it following the 6 steps provided above.
}
