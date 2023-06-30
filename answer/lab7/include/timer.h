#pragma once
void timer_init();
void handel_timer_1();
void arm_core_timer_intr_handler();
void add_timer(void (*callback_f)(),int times);
void sleep(int duration);
void core_timer_init();
void thread_timer();