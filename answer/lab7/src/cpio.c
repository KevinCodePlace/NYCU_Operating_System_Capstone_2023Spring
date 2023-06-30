#include "uint.h"
#include "mini_uart.h"
#include "string.h"
#include "excep.h"
#include "thread.h"
#include "queue.h"
#include "scheduler.h"
#include "task.h"
#include "mmu.h"
#include "vfs.h"
#include "cpio.h"
#include "list.h"
extern uint64_t cpio_start;
struct file_operations *cpio_fops;
struct vnode_operations *cpio_vops;
struct vnode* cpio_root = NULL;

typedef struct cpio_newc_header {  //cpio new ascii struct
		   char	   c_magic[6];
		   char	   c_ino[8];
		   char	   c_mode[8];
		   char	   c_uid[8];
		   char	   c_gid[8];
		   char	   c_nlink[8];
		   char	   c_mtime[8];
		   char	   c_filesize[8];
		   char	   c_devmajor[8];
		   char	   c_devminor[8];
		   char	   c_rdevmajor[8];
		   char	   c_rdevminor[8];
		   char	   c_namesize[8];
		   char	   c_check[8];
}CPIO_H ;

struct vnode* cpio_create_node(const char *name,const char mode, size_t size,const char *internal){
    struct vnode* new_node = allo_vnode();
    strcpy(name,new_node->dt,32);
    new_node->f_ops = cpio_fops;
    new_node->v_ops = cpio_vops;
    new_node->size = size;
    new_node->internal = internal;

    new_node->dt->vnode = new_node;
    if(mode == '4'){
        new_node->dt->type = directory;
        new_node->dt->childs = malloc(sizeof(struct link_list));
        delete_last_mem();
        new_node->dt->childs->next = NULL;
        struct dentry *tmpchild = allo_dentry();
        tmpchild->name[0] = '.';
        tmpchild->name[1] = 0;
        tmpchild->type = directory;
        tmpchild->vnode = new_node;
        tmpchild->childs = new_node->dt->childs;
        new_node->dt->childs->entry = tmpchild;
    }else{
        new_node->dt->type = file;
        new_node->dt->vnode = new_node;
    }
    return new_node;
}

void traverse_cpio(struct vnode* root){ // proceed ls
    uint64_t addr = cpio_start;
    CPIO_H *cpio=(CPIO_H*) addr;
    while(strncmp(cpio->c_magic,"070701\0",6)==1){ //c_magic is always "070701"
        struct vnode* parent = root;
        int namesize=a16ntoi(cpio->c_namesize, 8);
        char *name=(char*)(cpio+1);
        char temp[128];
        memset(temp,0,128);
        for(int i=0,j=0;i<namesize;i++){
            if(name[i]=='/'){
                struct vnode* tmp;
                cpio_lookup(parent,&tmp,temp);
                parent = tmp;
                memset(temp,0,128);
                j = 0;
            }else temp[j++]=name[i];
        }
        if(strncmp(temp, "TRAILER!!!", 10)) return;
        int filesize=a16ntoi(cpio->c_filesize, 8);


        if(temp[0]!='.'){
            struct vnode* new_node = cpio_create_node(temp,cpio->c_mode[4],filesize,addr+110+namesize); 
            if(cpio->c_mode[4] == '4'){
                new_node->dt->parent = parent->dt;
                struct link_list* parent_list = malloc(sizeof(struct link_list));
                delete_last_mem();
                parent_list->next = new_node->dt->childs->next;
                new_node->dt->childs->next = parent_list;
                struct dentry* parent_dentry = allo_dentry();
                strcpy("..\0",parent_dentry->name,32);
                parent_dentry->type = directory;
                parent_dentry->vnode = parent;
                parent_dentry->parent = parent->dt->parent;
                parent_dentry->childs = parent->dt->childs;
                parent_list->entry = parent_dentry;
            }
            struct link_list* ll = parent->dt->childs;
            //uart_printf("%s\n%s\n",parent->dt->name,new_node->dt->name);
            while(ll->next != NULL) ll = ll->next;
            ll->next = malloc(sizeof(struct link_list));
            delete_last_mem();
            ll->next->entry = new_node->dt;
            ll->next->next = NULL;
            //uart_printf("13");
        }
        int sum=110+namesize+filesize;
        if(sum%4 != 0){
            sum/=4;
            sum*=4;
            sum+=4;
        } 
        addr+=sum;  // padding is included in namesize and filesize

        cpio=(CPIO_H*) addr;
    }
}

uint64_t find_file_addr(char *file_name){
    uint64_t addr = cpio_start;
    CPIO_H *cpio=(CPIO_H*) addr;
    while(strncmp(cpio->c_magic,"070701\0",6)==1){
        int namesize=a16ntoi(cpio->c_namesize, 8);
        char *name=(char*)(cpio+1);
        char temp[128];
        for(int i=0;i<namesize;i++){
            temp[i]=name[i];
        }
        temp[namesize]='\0';
        if(strncmp(temp, "TRAILER!!!", 10)) break;
        int filesize=a16ntoi(cpio->c_filesize, 8);
        if(strcmp(file_name, temp)){
            return addr;
        }
        int sum=110+namesize+filesize;
        if(sum%4 != 0){
            sum/=4;
            sum*=4;
            sum+=4;
        }
        addr+=sum;  // padding is included in namesize and filesize
        cpio=(CPIO_H*) addr;
    }
    return NULL;
}

void* getContent(uint64_t addr,int *length){
    CPIO_H *cpio=(CPIO_H*) addr;
    int namesize=a16ntoi(cpio->c_namesize, 8);
    int filesize=a16ntoi(cpio->c_filesize, 8);
    addr += (110+namesize);
    *length = filesize;
    return addr;
}

void print_content(char *file){
    uint64_t f_addr=find_file_addr(file);
    if(f_addr != NULL){
        int length;
        char *content_addr = getContent(f_addr, &length);
        while(length--){
            if(*content_addr == '\n') uart_write('\r');
            uart_write(*content_addr++);
        }
        return;
    }else{
        uart_printf("Not found file \"%s\"\n",file);
        return;
    }
}

void copy_content(char *file,void **addr, uint64_t *size){
    struct vnode *tmp;
    vfs_lookup(file,&tmp);
    if(tmp != NULL){
        if(tmp->dt->type == directory){
            *addr = NULL;
            return;
        }
        int length = tmp->size;
        uint64_t address = tmp->internal;
        if(address%4 != 0){
            address /= 4;
            address *= 4;
            address += 4;
        }
        void * start = malloc(length);
        move_last_mem(0);
        byte *ucode = (uint64)start;
        byte *file = address;
        for(int i=0;i<length;i++){
            ucode[i] = file[i];
        }
        *addr = ucode;
        *size = length;
    }else{
        uart_printf("Not found file \"%s\"\n",file);
        return;
    }
}

void init_cpio(){
    cpio_vops = malloc(sizeof(struct vnode_operations));
    delete_last_mem();
    cpio_fops = malloc(sizeof(struct file_operations));
    delete_last_mem();
    cpio_vops->create   = cpio_perm_denied;
    cpio_vops->mkdir    = cpio_perm_denied;
    cpio_vops->lookup   = cpio_lookup;
    cpio_vops->mknod    = cpio_perm_denied;
    cpio_fops->write    = cpio_perm_denied;
    cpio_fops->close    = cpio_close;
    cpio_fops->lseek64  = cpio_lseek64;
    cpio_fops->open     = cpio_open;
    cpio_fops->read     = cpio_read;

    struct filesystem *fs = malloc(sizeof(struct filesystem));
    delete_last_mem();
    fs->name = malloc(16);
    delete_last_mem();
    memset(fs->name,0,16);
    strcpy("initramfs\0",fs->name,10);
    fs->setup_mount = cpio_mount;
    register_filesystem(fs);
    vfs_mount("/initramfs\0","initramfs\0");
    struct vnode* tmp;
    vfs_lookup("/initramfs\0",&tmp);
}

int cpio_mount(struct filesystem* fs, struct mount* mount){
    if(cpio_root == NULL){
        cpio_root = allo_vnode();
        cpio_root->mount = NULL;
        cpio_root->size = 0x400;
        cpio_root->f_ops = cpio_fops;;
        cpio_root->v_ops = cpio_vops;
        cpio_root->internal = NULL;

        struct dentry *tmp_child = allo_dentry();
        struct dentry *tmp_parent = mount->root->dt->childs->next->entry;

        strcpy(mount->root->dt->name[0],cpio_root->dt->name,256);
        cpio_root->dt->type = directory;
        cpio_root->dt->vnode = cpio_root;
        cpio_root->dt->parent = cpio_root->dt;
        cpio_root->dt->childs = malloc(sizeof(struct link_list));
        delete_last_mem();
        cpio_root->dt->childs->entry = tmp_child;
        cpio_root->dt->childs->next = malloc(sizeof(struct link_list));
        delete_last_mem();
        cpio_root->dt->childs->next->entry = tmp_parent;
        cpio_root->dt->childs->next->next= NULL;

        tmp_child->name[0] = '.';
        tmp_child->name[1] = 0;
        tmp_child->parent = cpio_root->dt;
        tmp_child->childs = cpio_root->dt->childs;
        tmp_child->type = directory;
        tmp_child->vnode = cpio_root;

    }
    mount->root = cpio_root;
    traverse_cpio(cpio_root);
    struct link_list* ll = cpio_root->dt->childs;
}

int cpio_perm_denied(){
    uart_printf("Error: Read only file\n");
    return -1;
}

int cpio_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct link_list* tmp = dir_node->dt->childs;
    while(tmp != NULL){
        struct dentry* den_tmp = tmp->entry;
        if(strcmp(den_tmp->name,component_name)){
            *target = den_tmp->vnode;
            return 0;
        }
        tmp = tmp->next;
    }
    *target = NULL;
    return -1;
}

int cpio_close(struct file* file){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
    free(file);
    return 0;
}

int cpio_read(struct file* file, void* buf, size_t len){
    uint64_t address = file->vnode->internal;
    char *internal, *tmp = buf;
    if(address%4 != 0){
        address /= 4;
        address *= 4;
        address += 4;
    }
    internal = address;
    for(int i=0;i<len;i++){
        tmp[i] = internal[file->f_pos++];
    }
    return len;
}

int cpio_open(struct vnode* file_node, struct file** target){
    struct file* tmp = malloc(sizeof(struct file));
    delete_last_mem();
    tmp->f_pos = 0;
    tmp->f_ops = cpio_fops;
    tmp->vnode = file_node;
    *target = tmp;
    return 0;
}

long cpio_lseek64(struct file* file, long offset, int whence){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
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