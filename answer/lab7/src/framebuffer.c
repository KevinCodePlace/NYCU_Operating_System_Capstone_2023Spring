#include "framebuffer.h"
#include "string.h"
#include "mini_uart.h"
#include "allocator.h"
#include "mailbox.h"
#include "irq.h"
struct file_operations *fb_fops;
struct vnode_operations *fb_vops;


int fb_setup(struct device* device, struct vnode* mount){
    mount->f_ops = fb_fops;
    mount->v_ops = fb_vops;
}

int fb_per_denied(){
    uart_printf("Permission denied\n");
    return -1;
}

int fb_write(struct file* file, const void* buf, size_t len){
    const unsigned char *color = buf;
    for(int i=0;i<len;i++){
        fb_splash(*(color+i),file->f_pos++);
    }
    return len;
}

int fb_open(struct vnode* file_node, struct file** target){
    struct file* tmp = malloc(sizeof(struct file));
    delete_last_mem();
    tmp->f_pos = 0;
    tmp->f_ops = fb_fops;
    tmp->vnode = file_node;
    *target = tmp;
    return 0;
}

int fb_close(struct file* file){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
    free(file);
    return 0;
}

int fb_lseek64(struct file* file, long offset, int whence){
    if(whence == SEEK_END){
        file->f_pos = file->vnode->size + offset;
        return file->f_pos;
    }else if(whence == SEEK_CUR){
        file->f_pos += offset;
        return file->f_pos;
    }else if(whence == SEEK_SET){
        file->f_pos = offset;
        return file->f_pos;
    }
}

int fb_ioctl(struct file* file, unsigned long request, uint32_t fb_info[4]){
    if(request == 0){
        set_vc(fb_info);
        return 0;
    }
    return -1;
}

void vfs_fb_init(){
    struct device *dev = malloc(sizeof(struct device));
    delete_last_mem();
    dev->name = malloc(16);
    delete_last_mem();
    memset(dev->name,0,16);

    fb_fops = malloc(sizeof(struct file_operations));
    fb_vops = malloc(sizeof(struct vnode_operations));
    delete_last_mem();
    delete_last_mem();
    fb_fops->open     = fb_open;
    fb_fops->read     = fb_per_denied;
    fb_fops->write    = fb_write;
    fb_fops->close    = fb_close;
    fb_fops->lseek64  = fb_lseek64;
    fb_vops->create   = fb_per_denied;
    fb_vops->lookup   = fb_per_denied;
    fb_vops->mkdir    = fb_per_denied;
    fb_vops->mknod    = fb_per_denied;
    fb_vops->ioctl    = fb_ioctl;



    strcpy("framebuffer\0",dev->name,12);
    dev->setup = fb_setup;
    register_device(dev);
    vfs_mknod("/dev/framebuffer", "framebuffer");
}