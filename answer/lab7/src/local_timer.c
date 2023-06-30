#include "uint.h"
#include "mini_uart.h"
#include "peripheral/irq.h"
#include "aux.h"
#include "peripheral/timer.h"

uint32 interval_time = CLOCKHZ * 2;
uint32 cur_val_1 = 0;

void timer_init(){
    cur_val_1 = *SYSTEM_TIMER_COUNTER_L_32;
    cur_val_1 += interval_time;
    *SYSTEM_TIMER_COMPARE1 = cur_val_1;
}

void handle_timer_1(){
    cur_val_1 += interval_time;
    *SYSTEM_TIMER_COMPARE1 = cur_val_1;
    *SYSTEM_TIMER_CONTROL |= 2;

    uart_printf("Timer 1 recceived!\n");
}