#include "mini_uart.h"
#include "dtb.h"
#include "exception_c.h"
#include "utils_s.h"
#include "shell.h"
#include "mm.h"
#include "timer.h"
#include "thread.h"
#include "sche.h"
#include "exec.h"

static void idle(void)
{
    while (1)
    {
        thread_kill_zombies();
        thread_schedule(0);
    }
}

void kernel_main(void)
{
	uart_init();
    uart_send_string("Hello, world!\n");
    mm_init();
    thread_init();
    // thread_create(&shell);
    exe_new_prog("syscall.img");
    timeout_event_init();
    add_timer((timer_callback)thread_schedule, (size_t)0, MS(SCHE_CYCLE));
    enable_interrupt();
    idle();
}
