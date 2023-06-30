#pragma once
#include "vfs.h"
#define EOF 0
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
void init_cpio();
void traverse_cpio(struct vnode* root);
void print_content(char *file);
void copy_content(char *file,void **addr, uint64_t *size);
void* getContent(uint64_t addr,int *length);
void init_cpio();
int cpio_mount(struct filesystem* fs, struct mount* mount);
int cpio_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name);
int cpio_read(struct file* file, void* buf, size_t len);
int cpio_open(struct vnode* file_node, struct file** target);
int cpio_close(struct file* file);
long cpio_lseek64(struct file* file, long offset, int whence);
int cpio_perm_denied();