#include "aux.h"
#include "gpio.h"
#include "buffer.h"
#include "allocator.h"
#include "jump.h"
#include "signal.h"
#include "thread.h"
#include "vfs.h"
#include "string.h"

int transmit_interrupt_open = 0;
char uart_buffer[1024];
unsigned int wr_buffer_index = 0;
unsigned int rd_buffer_index = 0;
struct buffer wbuffer,rbuffer;
extern struct jumpBuf jb;


void uart_init() {

    /* Initialize UART */
    *AUX_IRQ |= 1;       // Enable mini UART interrupt pending
    *AUX_ENABLES |= 1;   // Enable mini UART
    *AUX_MU_CNTL = 0;    // Disable TX, RX during configuration
    *AUX_MU_IER = 1;     // enable interrupt
    *AUX_MU_LCR = 3;     // Set the data size to 8 bit
    *AUX_MU_MCR = 0;     // Don't need auto flow control
    *AUX_MU_BAUD = 270;  // Set baud rate to 115200
    *AUX_MU_IIR = 6;     // No FIFO

    /* Map UART to GPIO Pins */

    // 1. Change GPIO 14, 15 to alternate function
    register unsigned int r = *GPFSEL1;  //gpio10~19 is located GPSEL1
    r &= ~((7 << 12) | (7 << 15));  // Reset GPIO 14, 15 
    r |= (2 << 12) | (2 << 15);     // Set ALT5
    *GPFSEL1 = r;

    // 2. Disable GPIO pull up/down (Because these GPIO pins use alternate functions, not basic input-output)
    // Set control signal to disable
    *GPPUD = 0;
    // Wait 150 cycles
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // Clock the control signal into the GPIO pads
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    // Wait 150 cycles
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // Remove the clock
    *GPPUDCLK0 = 0;

    // 3. Enable TX, RX
    *AUX_MU_CNTL = 3;

    transmit_interrupt_open = 0;
}

void uart_init_buffer(){
    wbuffer.start = 0;
    wbuffer.end = 0;
    rbuffer.start = 0;
    rbuffer.end = 0;
}

char uart_read() {
    // Check data ready field
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    // Read
    char r = (char)(*AUX_MU_IO);
    // Convert carrige return to newline
    return r == '\r' ? '\n' : r;
}

char uart_read_raw() {
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    return (char)(*AUX_MU_IO);
}

void uart_write(unsigned int c) {
    // Check transmitter idle field
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    // Write
    *AUX_MU_IO = c;
}

void uart_printf(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '\n') uart_write('\r');
        else if(*fmt == '%'){
            fmt++;
            if(*fmt == 'd'){
                int arg =  __builtin_va_arg(args, int);
                char temp[10];
                itoa(arg,temp);
                uart_printf(temp);
            }else if(*fmt == 's'){
                char *arg =  __builtin_va_arg(args, char*);
                uart_printf(arg);
            }else if(*fmt == 'x'){
                int arg =  __builtin_va_arg(args, int);
                char temp[10];
                i16toa(arg,temp,8);
                uart_printf(temp);
            }else if(*fmt == 'c'){
                unsigned char arg = __builtin_va_arg(args,int);
                uart_write(arg);
            }else if(*fmt == '.'){
                int l = *(++fmt)-'0';
                if(*(++fmt) == 'f'){
                    float *arg = __builtin_va_arg(args, float*);
                    char temp[20];
                    ftoa(arg,l,temp);
                    uart_printf(temp);
                }            
            }
            else if(*fmt++ == 'l'){
                if(*fmt == 'e'){
                    unsigned int arg = __builtin_va_arg(args, unsigned int);
                    unsigned char *t = (unsigned char *) &arg;
                    for(int i=0;i<4;i++){
                        unsigned char temp[3];
                        temp[2]='\0';
                        i16toa(t[i],temp,2);
                        uart_printf(temp);
                    }
                }else if(*fmt == 'l'){
                    unsigned long long arg = __builtin_va_arg(args, unsigned long long);
                    unsigned char temp[24];
                    itoa(arg, temp);
                    uart_printf("%s",temp);
                }
            }
            fmt++;
            continue;
        }
        else if(!(*fmt >=32 && *fmt <= 127)) break;
        uart_write(*fmt++);
    }
}

void uart_flush() {
    while (*AUX_MU_LSR & 0x01) {
        *AUX_MU_IO;
    }
}

int uart_push(char c){
    return write_buffer(&wbuffer,c);
}

int uart_pop(unsigned char *c){
    return read_buffer(&rbuffer,c);
}

void *handle_uart_irq()
{
    unsigned int id = *AUX_MU_IIR;
    if((id & 0x06) == 0x04)     //receive interrupt
	{
        if( *AUX_MU_LSR & 0x01) {
            char c;
            c = *AUX_MU_IO & 0xFF;
            if(c == 3){
                uart_printf("^C\n");
                reset_flag();
                
                longjump(&jb,1);
            }else{
                write_buffer(&rbuffer,c);
            }
        }
	}
    if((id & 0x06) == 0x02)   //transmit interrupt
	{
        while(*AUX_MU_LSR & 0x20) {
            unsigned char c;
            if(read_buffer(&wbuffer,&c) == 0) {
                // close transmit interrupt
                *AUX_MU_IER = 1; 
                transmit_interrupt_open = 0;
                return;
            }
            *AUX_MU_IO = c;
        }
	}
    *AUX_MU_IER = 1;
    return;
}

struct file_operations *uart_fops;
struct vnode_operations *uart_vops;


int uart_setup(struct device* device, struct vnode* mount){
    mount->f_ops = uart_fops;
    mount->v_ops = uart_vops;
}

int device_per_denied(){
    uart_printf("Permission denied\n");
    return -1;
}

int uart_vfs_write(struct file* file, const void* buf, size_t len){
    char *tmp = buf;
    //for(int i=0;i<len && tmp[i]!=0;i++){
        uart_printf(tmp);
    //}
    return len;
}

int uart_vfs_read(struct file* file, void* buf, size_t len){
    char *internal = buf;
    char temp[256];
    int rn = 0;
    while(1){
        temp[0] = 0;
        uart_pop(temp);
        for(int i=0;i<256;i++){
            if(temp[i] == 0) break;
            internal[rn++] = temp[i];
            if(rn >= len){
                return len;
            }
        }
    }
}

int uart_open(struct vnode* file_node, struct file** target){
    struct file* tmp = malloc(sizeof(struct file));
    delete_last_mem();
    tmp->f_pos = 0;
    tmp->f_ops = uart_fops;
    tmp->vnode = file_node;
    *target = tmp;
    return 0;
}

int uart_close(struct file* file){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
    free(file);
    return 0;
}

void vfs_uart_init(){
    struct device *dev = malloc(sizeof(struct device));
    delete_last_mem();
    dev->name = malloc(16);
    delete_last_mem();
    memset(dev->name,0,16);

    uart_fops = malloc(sizeof(struct file_operations));
    uart_vops = malloc(sizeof(struct vnode_operations));
    delete_last_mem();
    delete_last_mem();
    uart_fops->open     = uart_open;
    uart_fops->read     = uart_vfs_read;
    uart_fops->write    = uart_vfs_write;
    uart_fops->close    = uart_close;
    uart_fops->lseek64  = device_per_denied;
    uart_vops->create   = device_per_denied;
    uart_vops->lookup   = device_per_denied;
    uart_vops->mkdir    = device_per_denied;
    uart_vops->mknod    = device_per_denied;



    strcpy("uart\0",dev->name,5);
    dev->setup = uart_setup;
    register_device(dev);
    vfs_mknod("/dev/uart", "uart");
}

