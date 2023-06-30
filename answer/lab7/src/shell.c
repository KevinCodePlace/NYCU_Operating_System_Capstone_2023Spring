#include "mini_uart.h"
#include "string.h"
#include "reboot.h"
#include "uint.h"
#include "dtb.h"
#include "allocator.h"
#include "aux.h"
#include "thread.h"
#include "getopt.h"
#include "scheduler.h"
#include "loadimg.h"
#include "mailbox.h"
#include "mmu.h"
#include "vfs.h"
#include "tmpfs.h"
#include "framebuffer.h"

struct ARGS{
    char** argv;
    int argc;
};


extern unsigned char __heap_start, _end_, _begin_;
extern byte __dtb_addr;
uint64_t cpio_start,cpio_end;
char cmd_buffer[1024];
unsigned int cmd_index = 0;
unsigned int cmd_flag  = 0;


void shell_init(){
    uint64 *heap = (uint64*)(&__heap_start-8);
    *heap &= 0x00000000;
    uart_init();
    uart_printf("\n\n\nHello From RPI3\n");
    uart_init_buffer();
    uart_flush();
    core_timer_init();
    init_allocator();
    uint64 *ramf_start,*ramf_end;
    ramf_start=find_property_value("/chosen\0","linux,initrd-start\0");  //get ramf start addr from dtb
    ramf_end=find_property_value("/chosen\0","linux,initrd-end\0"); //get ramf end addr from dtb
    if(ramf_start != 0){
        uart_printf("Ramf start: 0x%x\n",letobe(*ramf_start));
        cpio_start=letobe(*ramf_start);
    }if(ramf_end != 0){
        uart_printf("Ramf end: 0x%x\n",letobe(*ramf_end));
        cpio_end=letobe(*ramf_end);
    }
    cpio_start |= 0xFFFF000000000000;
    cpio_end |= 0xFFFF000000000000;
    memory_reserve(0xffff000000000000,0xffff000000080000);  //Spin tables for multicore boot

    memory_reserve(0xFFFF000000001000,0xFFFF000000003000);  //Spin tables for multicore boot
    
    memory_reserve(cpio_start,cpio_end);  //Initramfs

    memory_reserve(&_begin_,&_end_);  //Kernel image in the physical memory and simple allocator

    uint64 *addr = &__dtb_addr;

    memory_reserve(*addr,*addr + 0x100000); //Device tree

    memory_reserve(0xFFFF000000000000,0xFFFF000000080000);  //Kernel stack

    init_thread();

    cow_init();

    vfs_init();

    tmpfs_init();

    init_cpio();

    vfs_uart_init();

    vfs_fb_init();

}

void reset_flag(){
    cmd_flag=0;
    *AUX_MU_IER = 1;
    core_timer_disable();
    irq_enable();
}

void uart_read_line(){
    //check("./initramfs/vfs1.img");
    char in;
    if(cmd_flag == 0){
        uart_printf("# ");
        cmd_flag = 1;
        cmd_index = 0;
        for(int i=0;i<1024;i++) cmd_buffer[i]=0;
    }
    while( uart_pop(&in) ){
        if(cmd_flag == 0){
            uart_printf("# ");
            cmd_flag = 1;
            cmd_index = 0;
            for(int i=0;i<1024;i++) cmd_buffer[i]=0;
        }
        if(in == 13){
            uart_printf("\n");
            cmd_buffer[cmd_index++] = '\0';
            cmd_flag = 0;
            check(cmd_buffer);
        }else if((in==8 || in==127)){
            if(cmd_index>0){
                cmd_index--;
                char t[1]={8};
                cmd_buffer[cmd_index]='\0';
                uart_write(8);
                uart_write(' ');
                uart_write(8);
            }
        }else if( in>=32 && in<=126 ){
            cmd_buffer[cmd_index++]=in;
            uart_push(in);
            *AUX_MU_IER |= 2;
        }
    }
}

void* m[10];

struct ARGS* parse_command(char *command){
    char** tmp = malloc(sizeof(char*)*16);
    int p=0;
    if(command[0] == 0) return NULL;
    for(int i=0;command[i] != 0;i++){
        if(command[i] == ' ') continue;
        char *arg = malloc(sizeof(char[128]));
        int p2=0;
        for(;command[i] != 0 && command[i] != ' ' && command[i]>=32 && command[i]<=127 ;i++){
            arg[p2++] = command[i];
        }
        arg[p2] = 0;
        tmp[p++] = arg;
        if(command[i] == 0) break;
    }
    struct ARGS* args;
    args = malloc(sizeof(struct ARGS));
    args->argc = p;
    args->argv = tmp;
    return args;
}

void temp_func2(){
    uart_printf("2");
}

void temp_func3(){
    uart_printf("3");
}
void temp_func(){
}



void fork_test(){
    uart_printf("\nFork Test, pid %d\n", getpid1());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork1()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid1(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork1()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            uart_printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid1(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                uart_printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid1(), cnt, &cnt, cur_sp);
                delay(5000);
                ++cnt;
            }
        }
        exit1();
    }
    else {
        uart_printf("parent here, pid %d, child %d\n", getpid1(), ret);
    }
}

void check(char *input){
    struct ARGS *cmd = parse_command(input);
    if(input[0] == '\0' || input[0] == '\n') return;
    if(strcmp(input,"help")==1){
        uart_printf("help    : print the help menu\n");
        uart_printf("hello   : print Hello World!\n");
        uart_printf("reboot  : reboot the device\n");
    }else if(strcmp(input,"hello")==1){
        uart_printf("Hello World!\n");
    }else if(strncmp(input,"reboot",6)==1){
        uart_printf("Rebooting...\n");
        if(input[6] != ' '){
            reset(50);
            while(1);
        }
        int a=0;
        for(int i=7;input[i]<='9' && input[i]>='0';i++){
            a *= 10;
            a += (input[i]-'0');
        }
        reset(a<50? 50:a);
        while(1);
    }else if(strncmp(input,"ls",2)){
        char name[128];
        memset(name,0,128);
        int i=3;
        for(;input[i]>=46 && input[i]<=122  && i<128 && input[i]!='\0'; i++){
            name[i-3]=input[i];
        }
        name[i]='\0';
        vfs_ls(name);
    }else if(strncmp(input,"cd ",3)){
        char name[128];
        memset(name,0,128);
        int i=3;
        for(;input[i]>=46 && input[i]<=122  && i<128 && input[i]!='\0'; i++){
            name[i-3]=input[i];
        }
        name[i]='\0';
        vfs_cd(name);
    }else if(strncmp(input,"cat ", 4)){
        char name[128];
        for(int i=0;i<128;i++) name[i] &= 0;
        int i=4;
        for(i=4;input[i]>=46 && input[i]<=122  && i<128 && input[i]!='\0'; i++){
            name[i-4]=input[i];
        }
        name[i]='\0';
        print_content(name, cpio_start);
    }else if(strncmp(input,"./",2)){
        irq_disable();
        char name[128];
        for(int i=0;i<128;i++) name[i] = 0;
        int i;
        for(i=2;input[i]>=46 && input[i]<=122  && i<128 && input[i]!='\0'; i++){
            name[i-2]=input[i];
        }
        char *const argv[] = {name};
        execute(name,argv);
        irq_enable();
    }else if(strcmp(input,"timer")){
        int clock_hz,now_time,interval;
        asm volatile("mrs %[input0], cntfrq_el0\n"
                     "mrs %[input1], cntp_tval_el0\n"
                     :[input0] "=r" (clock_hz),
                     [input1] "=r" (interval));
        uart_printf("%d\n", interval/clock_hz);
        
    }else if(strncmp(input,"sleep", 5)){
        char time[5];
        for(int i=0;i<5 && input[i+6]>=32 && input[i+6]<=127;i++) time[i] = input[i+6];
        int times = atoi(time);
        sleep(times);
    }else if(strcmp(cmd->argv[0],"thread")){
        int t=1;
        optind = 1;
        while(t){
            char c = getopt(cmd->argc,cmd->argv,":a:r");
            switch (c){
                case 'a':
                    for(int i=0;i<10;i++)
                        Thread(temp_func);
                    break;
                case 'r':
                    break;
                case 0:
                    t=0;
                    break;
            }
        }
    }else if(strcmp(cmd->argv[0],"mem")){
        int t=1;
        optind = 1;
        while(t){
            char c = getopt(cmd->argc,cmd->argv,":s:a:p");
            switch (c){
                case 's':
                    pool_status();
                    break;
                case 'a':
                    printf_thread();
                    break;
                case 'p':
                    print_node();
                    break;
                case 0:
                    t=0;
                    break;
                default:
                    show_status();
                    break;

            }
        }
    }else if(strcmp(cmd->argv[0],"lp")){
        loadimg();
    }else if(strcmp(cmd->argv[0],"test")){
        fb_splash2();
    }else{
        uart_printf("command not found: %s\n",input);
    }
}

