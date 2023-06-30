#include "exec.h"
#include "_cpio.h"
#include "sche.h"
#include "mm.h"
#include "trap_frame.h"
#include "utils_c.h"
#include "current.h"
#include "thread.h"
#include "mini_uart.h"
#include "vfs.h"
#include "mmu.h"

static void replace_user_context(void *prog, size_t data_size, pd_t *ttbr0)
{
    struct task *_task = current;

    kfree(_task->user_stack);
    free_page_table(_task->ttbr0);
    kfree(_task->user_prog);

    _task->user_stack = alloc_stack(ttbr0, STACK_SIZE);

    _task->user_prog = prog;
    _task->user_prog_size = data_size;

    _task->ttbr0 = ttbr0;
    replace_page_table();

    TrapFrame *trapframe = (TrapFrame *)((char *)_task->kernel_stack + STACK_SIZE - sizeof(TrapFrame));
    memset(trapframe, 0, sizeof(TrapFrame));
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
    jump_user_prog((char *)UPROG_VA, 0, (char *)USTACK_VA + (STACK_SIZE - 0x10));
}

int do_exec(const char *path, const char *argv[])
{
    pd_t *ttbr0 = kcalloc(PAGE_SIZE);
    struct file *file;
    if (vfs_open(path, 0, &file))
    {
        uart_send_string("[do_exec] fail to lookup\n");
    }
    size_t data_size = file->vnode->content_size;
    if (data_size == -1)
    {
        uart_send_string("!! do_exec fail !!\n");
        return -1;
    }
    void *target_addr = alloc_prog(ttbr0, data_size, file->vnode->content);

    setup_peripheral_identity(ttbr0);
    replace_user_context(target_addr, data_size, ttbr0);
    jump_user_prog((char *)UPROG_VA, current->kernel_stack + STACK_SIZE - sizeof(TrapFrame), (char *)USTACK_VA + (STACK_SIZE - 0x10));
    return 0;
}

void exe_new_prog(char *path)
{
    pd_t *ttbr0 = kcalloc(PAGE_SIZE);
    setup_peripheral_identity(ttbr0);

    struct file *file;
    if (vfs_open(path, 0, &file))
    {
        uart_send_string("[exe_new_prog] fail to lookup\n");
    }
    size_t data_size = file->vnode->content_size;
    if (data_size == -1)
    {
        uart_send_string("[exe_new_prog] data_size==-1\n");
        return ;
    }
    void *target_addr = alloc_prog(ttbr0, data_size, file->vnode->content);

    struct task *prog = thread_create(init_user_prog);

    prog->user_stack = alloc_stack(ttbr0, STACK_SIZE);

    prog->user_prog = target_addr;
    prog->user_prog_size = data_size;

    prog->ttbr0 = ttbr0;

    asm(
        "dsb ish\n"           // ensure write has completed
        "msr ttbr0_el1, %0\n" // switch translation based address.
        "tlbi vmalle1is\n"    // invalidate all TLB entries
        "dsb ish\n"           // ensure completion of TLB invalidatation
        "isb\n"               // clear pipeline
        ::"r"(ttbr0));

    return;
}
