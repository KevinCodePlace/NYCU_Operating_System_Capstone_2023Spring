#include "vfs.h"
#include "allocator.h"
#include "string.h"
#include "tmpfs.h"

struct file_operations *tmpfs_fops;
struct vnode_operations *tmpfs_vops;

extern struct mount* rootfs;

void tmpfs_init(){
    tmpfs_vops = malloc(sizeof(struct vnode_operations));
    delete_last_mem();
    tmpfs_fops = malloc(sizeof(struct file_operations));
    delete_last_mem();
    tmpfs_vops->create = tmpfs_create;
    tmpfs_vops->lookup = tmpfs_lookup;
    tmpfs_vops->mkdir = tmpfs_mkdir;
    tmpfs_vops->mknod = tmpfs_mknod;
    tmpfs_fops->close = tmpfs_close;
    tmpfs_fops->lseek64 = tmpfs_lseek64;
    tmpfs_fops->open = tmpfs_open;
    tmpfs_fops->read = tmpfs_read;
    tmpfs_fops->write = tmpfs_write;
    struct filesystem *fs = malloc(sizeof(struct filesystem));
    delete_last_mem();
    fs->name = malloc(16);
    delete_last_mem();
    memset(fs->name,0,16);
    strcpy("tmpfs\0",fs->name,6);
    fs->setup_mount = tmpfs_mount;
    register_filesystem(fs);
    fs->setup_mount(fs,rootfs);
    struct vnode* tmp;
    
    tmpfs_mkdir(rootfs->root,&tmp,"initramfs");
    tmpfs_mkdir(rootfs->root,&tmp,"dev");
}

int tmpfs_mount(struct filesystem* fs, struct mount* mount){
    mount->fs = fs;
    struct vnode *tmp = allo_vnode();
    struct dentry *tmp_child = allo_dentry(), *tmp_parent = allo_dentry();
    tmp->f_ops = tmpfs_fops;
    tmp->v_ops = tmpfs_vops;
    tmp->mount = mount;

    tmp->dt->name[0] = 0;
    tmp->dt->childs = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->childs->entry = tmp_child;
    tmp->dt->childs->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->vnode = tmp;
    tmp->dt->type = directory;

    tmp_child->name[0] = '.';
    tmp_child->name[1] = 0;
    tmp_child->vnode = tmp;
    tmp_child->childs = tmp->dt->childs;
    tmp_child->type = directory;
    tmp_child->parent = tmp->dt->parent;

    if(mount->root !=NULL){
        tmp->dt->parent = mount->root->dt->parent;
        tmp_parent = mount->root->dt->childs->next->entry;
    }else{
        tmp->dt->parent = tmp->dt;

        tmp_parent = allo_dentry();
        tmp_parent->name[0] = '.';
        tmp_parent->name[1] = '.';
        tmp_parent->name[2] = 0;
        tmp_parent->vnode = tmp;
        tmp_parent->childs = tmp->dt->childs;
        tmp_parent->type = directory;
        tmp_parent->parent = tmp->dt->parent;
    }
    
    tmp->dt->childs->next->entry = tmp_parent;
    tmp->dt->childs->next->next = NULL;

    mount->root = tmp;
    return 0;
}

int tmpfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct vnode* tmp;
    tmpfs_lookup(dir_node,&tmp,component_name);
    if(tmp != NULL){
        *target = NULL;
        return -1;
    }
    tmp = allo_vnode();
    strcpy(component_name,tmp->dt->name,16);
    tmp->f_ops = tmpfs_fops;
    tmp->internal = malloc(0x100);
    delete_last_mem();
    memset(tmp->internal,0,0x100);
    tmp->v_ops = tmpfs_vops;
    tmp->f_ops = tmpfs_fops;
    tmp->size = 0;

    tmp->dt->parent = dir_node->dt;
    tmp->dt->childs = NULL;
    tmp->dt->vnode = tmp;
    tmp->dt->type = file;
    *target = tmp;

    struct link_list *parent_ll = dir_node->dt->childs;
    while(parent_ll->next!=NULL)parent_ll = parent_ll->next;
    parent_ll->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    parent_ll->next->entry = tmp->dt;
    parent_ll->next->next = NULL;
    return 0;
}

int tmpfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct vnode* temp;
    tmpfs_lookup(dir_node,&temp,component_name);
    if(temp != NULL){
        *target = NULL;
        return -1;
    }
    struct vnode *tmp = allo_vnode();
    struct dentry *tmp_child = allo_dentry(),*tmp_parent = allo_dentry();
    
    strcpy(component_name,tmp->dt->name,16);
    tmp->f_ops = tmpfs_fops;
    tmp->v_ops = tmpfs_vops;
    tmp->size = 0;

    tmp->dt->parent = dir_node->dt;
    tmp->dt->childs = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->childs->entry = tmp_child;
    tmp->dt->childs->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp->dt->childs->next->entry = tmp_parent;
    tmp->dt->childs->next->next = NULL; 
    tmp->dt->vnode = tmp;
    tmp->dt->type = directory;
    *target = tmp;

    tmp_child->name[0] = '.';
    tmp_child->name[1] = 0;
    tmp_child->vnode = tmp;
    tmp_child->childs = tmp->dt->childs;
    tmp_child->type = directory;
    tmp_child->parent = tmp->dt->parent;

    tmp_parent->name[0] = '.';
    tmp_parent->name[1] = '.';
    tmp_parent->name[2] = 0;
    tmp_parent->vnode = tmp->dt->parent->vnode;
    tmp_parent->childs = tmp->dt->parent->childs;
    tmp_parent->type = directory;
    tmp_parent->parent = tmp->dt->parent->parent;

    struct link_list *tmp_parent_child = tmp_parent->childs;
    while(tmp_parent_child->next != NULL) tmp_parent_child = tmp_parent_child->next;
    tmp_parent_child->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    tmp_parent_child->next->next = NULL;
    tmp_parent_child->next->entry = tmp->dt;

    return 0;
}

int tmpfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name){
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

int tmpfs_write(struct file* file, const void* buf, size_t len){
    char *internal = file->vnode->internal, *tmp = buf;
    for(int i=0;i<len;i++){
        internal[file->f_pos++] = tmp[i];
        //if(tmp[i] == EOF) return i;
    }
    return len;
}

int tmpfs_read(struct file* file, void* buf, size_t len){
    char *internal = file->vnode->internal, *tmp = buf;
    for(int i=0;i<len;i++){
        tmp[i] = internal[file->f_pos++];
        if(tmp[i] == EOF) return i;
    }
    return len;
}

int tmpfs_open(struct vnode* file_node, struct file** target){
    struct file* tmp = malloc(sizeof(struct file));
    delete_last_mem();
    tmp->f_pos = 0;
    tmp->f_ops = tmpfs_fops;
    tmp->vnode = file_node;
    *target = tmp;
    return 0;
}

int tmpfs_close(struct file* file){
    if(file->f_pos > file->vnode->size) file->vnode->size = file->f_pos; 
    free(file);
    return 0;
}

long tmpfs_lseek64(struct file* file, long offset, int whence){
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

int tmpfs_mknod(struct vnode* dir_node, struct vnode** target, const char* component_name){
    struct vnode* tmp;
    tmpfs_lookup(dir_node,&tmp,component_name);
    if(tmp != NULL){
        *target = NULL;
        return -1;
    }
    tmp = allo_vnode();
    strcpy(component_name,tmp->dt->name,16);
    tmp->size = 0;
    tmp->dt->type = device;
    tmp->dt->vnode = tmp;
    tmp->mount = NULL;
    *target = tmp;

    struct link_list *parent_ll = dir_node->dt->childs;
    while(parent_ll->next!=NULL)parent_ll = parent_ll->next;
    parent_ll->next = malloc(sizeof(struct link_list));
    delete_last_mem();
    parent_ll->next->entry = tmp->dt;
    parent_ll->next->next = NULL;
    return 0;
}
