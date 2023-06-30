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
#include "mmu.h"
#include "vfs.h"

static void idle(void)
{
    while (1)
    {
        thread_kill_zombies();
        thread_schedule(0);
    }
}

void kernel_main(void *_dtb_ptr)
{
    dtb_start=(uintptr_t)_dtb_ptr;
    uart_send_string("Hello, world!\n");
    
    mm_init();
    setup_kernel_space_mapping();
    
    fs_init();

    thread_init();
    // thread_create(&shell);
    exe_new_prog("/initramfs/vfs2.img");
    
    timeout_event_init();
    add_timer((timer_callback)thread_schedule, (size_t)0, MS(SCHE_CYCLE));
    
    enable_interrupt();
    idle();
}