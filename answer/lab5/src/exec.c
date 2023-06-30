#include "exec.h"
#include "_cpio.h"
#include "sche.h"
#include "mm.h"
#include "trap_frame.h"
#include "utils_c.h"
#include "current.h"
#include "thread.h"
#include "mini_uart.h"

static void replace_user_context(void *prog, size_t data_size)
{
    struct task *_task = current;

    memset(_task->user_stack, 0, STACK_SIZE);
    kfree(_task->user_prog);
    _task->user_prog = prog;
    _task->user_prog_size = data_size;

    TrapFrame *trapframe = (TrapFrame *)((char *)_task->kernel_stack + STACK_SIZE - sizeof(TrapFrame));
    memset(trapframe, 0, sizeof(TrapFrame));

    trapframe->sp = (unsigned long)_task->user_stack + STACK_SIZE - 0x10;
}

void jump_user_prog(void *target_addr, char *kernel_sp, char *user_sp)
{
    asm volatile("mov x0, 0  \n");
    asm volatile("msr spsr_el1, x0   \n"); // daif=0
    asm volatile("msr elr_el1, %0    \n" ::"r"(target_addr));
    asm volatile("msr sp_el0, %0    \n" ::"r"(user_sp));
    if (kernel_sp)
    {
        asm volatile("mov sp, %0    \n" ::"r"(kernel_sp));
    }
    asm volatile("eret   \n");
}

static void init_user_prog()
{
    jump_user_prog(current->user_prog, 0, (char *)current->user_stack + STACK_SIZE - 0x10);
}

int do_exec(const char *path, const char *argv[])
{
    void *target_addr;
    size_t data_size = cpio_load_program(path, &target_addr);
    if (data_size == -1)
    {
        uart_send_string("!! do_exec fail !!\n");
        return -1;
    }

    replace_user_context(target_addr, data_size);
    jump_user_prog(current->user_prog, current->kernel_stack + STACK_SIZE - sizeof(TrapFrame), current->user_stack + STACK_SIZE - 0x10);
    return 0;
}

void exe_new_prog(char *filename)
{
    void *target_addr;
    size_t data_size = cpio_load_program(filename, &target_addr);
    if (data_size == -1)
    {
        uart_send_string("!! exe_new_prog fail !!\n");
        return;
    }
    struct task *prog = thread_create(init_user_prog);
    prog->user_stack = kmalloc(STACK_SIZE);
    memset(prog->user_stack, 0, STACK_SIZE);

    prog->user_prog = target_addr;
    prog->user_prog_size = data_size;
    return;
}
