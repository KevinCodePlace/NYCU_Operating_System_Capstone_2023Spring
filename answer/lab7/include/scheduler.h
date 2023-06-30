#pragma once

void* switch_to(void *prev,void *next);
void* get_current();
int store_and_jump(void* addr, void (*call_fun)());
void set_current(void* addr);
void set_proc(void* addr);
void SwitchTo(void* addr);
void task_schedule(void *addr);
void call_exit();
void ret_to_sig_han(void *sp_addr);