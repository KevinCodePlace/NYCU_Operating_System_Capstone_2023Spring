#include "uint.h"
#include "vfs.h"
#include "allocator.h"
#include "cpio.h"
#include "string.h"
#include "list.h"
#include "mini_uart.h"
#include "thread.h"
#include "scheduler.h"

struct mount* rootfs;
struct link_list* filesystem_pool = NULL;
struct device *dv[100];


struct filesystem* find_filesystem(const char *name){
  struct link_list* fs_pool = filesystem_pool;
  while(fs_pool != NULL){
    struct filesystem *fs = fs_pool->entry;
    if(strcmp(fs->name,name)) return fs;
    fs_pool = fs_pool->next;
  }
  return NULL;
}

struct device* find_device(const char *name){
  for(int i=0;i<100;i++){
    if(strcmp(name,dv[i]->name)){
      return dv[i];
    }
  }
  return NULL;
}

void vfs_init(){
  rootfs = malloc(sizeof(struct mount));
  rootfs->root = NULL;
  for(int i=0;i<100;i++) dv[i] = NULL;
}

int register_filesystem(struct filesystem* fs) {
  // register the file system to the kernel.
  // you can also initialize memory pool of the file system here.
  struct link_list *tmp_pool;
  if(filesystem_pool == NULL){
    tmp_pool = malloc(sizeof(struct link_list));
    filesystem_pool = tmp_pool;
  }else{
    tmp_pool = filesystem_pool;
    while(tmp_pool->next != NULL) tmp_pool = tmp_pool->next;
    tmp_pool->next = malloc(sizeof(struct link_list));
    tmp_pool = tmp_pool->next;
  }
  tmp_pool->next = NULL;
  tmp_pool->entry = fs;
  return 0;
}

int register_device(struct device* d) {
  for(int i=0;i<100;i++){
    if(dv[i] == NULL){
      dv[i] = d;
      return 0;
    }
  }
  return -1;
}

int vfs_open(const char* pathname, int flags, struct file** target) {
  // 1. Lookup pathname
  // 2. Create a new file handle for this vnode if found.
  // 3. Create a new file if O_CREAT is specified in flags and vnode not found
  // lookup error code shows if file exist or not or other error occurs
  // 4. Return error code if fails
  
  struct vnode* filenode;
  int errno;
  if((errno = vfs_lookup(pathname,&filenode)) < 0){
    if(flags & O_CREAT){
      int last_slash = 0;
      for(int i=0;pathname[i]!=0 && i<256;i++){
        if(pathname[i] == '/') last_slash = i;
      }
      char filename[256];
      memset(filename,0,256);
      if(last_slash!=0)
        for(int i=0;i<last_slash;i++){
          filename[i] = pathname[i];
        }
      else filename[0] = '/';
      struct vnode* parent;
      
      if((errno = vfs_lookup(filename,&parent)) <0){
        return errno;
      }
      memset(filename,0,256);
      for(int i=last_slash+1;pathname[i]!=0 && i<256;i++){
        filename[i-last_slash-1] = pathname[i];
      }
      if((errno = parent->v_ops->create(parent,&filenode,filename)) <0 ){
        return errno;
      }
    }else return errno;
  }
  int return_value = filenode->f_ops->open(filenode,target);
  (*target)->flags = flags;
  return return_value;
}

int vfs_close(struct file* file) {
  // 1. release the file handle
  // 2. Return error code if fails
  return file->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  if(file == NULL) return -1;
  return file->f_ops->write(file,buf,len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
  // 1. read min(len, readable size) byte to buf from the opened file.
  // 2. block if nothing to read for FIFO type
  // 2. return read size or error code if an error occurs.
  if(file == NULL) return -1;
  file->f_pos = 0;
  return file->f_ops->read(file,buf,len);
}

int vfs_mkdir(const char* pathname){
  int last_slash = 0;
  char path[256], component[256];
  memset(path,0,256);
  memset(component,0,256);
  for(int i=0;i<256;i++){
    if(pathname[i] == '/' && pathname[i+1]!=0 && pathname[i+1]!=' ') last_slash = i;
    if(pathname[i] == 0) break;
  }
  for(int i=0;i<last_slash;i++){
    path[i] = pathname[i];
  }
  for(int i=last_slash+1;i<256;i++){
    if(pathname[i] == 0)break;
    component[i-last_slash-1] = pathname[i];
  }
  struct vnode* target, *target2;
  vfs_lookup(path,&target);
  int errno = target->v_ops->mkdir(target,&target2, component);
  if(errno >= 0) target2->mount = NULL; 
  return errno;
}

int vfs_mount(const char* target, const char* filesystem){
  struct vnode* mountpoint;
  int errno;
  if((errno = vfs_lookup(target,&mountpoint)) < 0){
    return errno;
  }
  struct filesystem *fs = find_filesystem(filesystem);
  if(fs == NULL) return -1;
  if(mountpoint->mount != NULL){
    return -1;
  }
  mountpoint->mount = malloc(sizeof(struct mount));
  mountpoint->mount->fs = fs;
  mountpoint->mount->root = malloc(sizeof(struct vnode));
  mountpoint->mount->root->mount = NULL;
  mountpoint->mount->root->dt = mountpoint->dt;
  
  return fs->setup_mount(fs,mountpoint->mount);
}

int vfs_lookup(const char* pathname, struct vnode** target){
  struct thread *t = get_current();
  struct vnode* CurrWorkDir = t->CurWorkDir;
  char *parse = malloc(16);
  memset(parse, 0, 16);
  int idx = 0;
  for(int i=0;i<256;i++){
    if(pathname[i] == '/'){
      if(i == 0){
        CurrWorkDir = rootfs->root;
      }
      else{
        if(CurrWorkDir->dt->type != directory) return -1;
        struct vnode* tmp;
        int errno;
        if((errno = CurrWorkDir->v_ops->lookup(CurrWorkDir,&tmp,parse)) < 0){
          return errno;
        }
        if(tmp->mount != NULL && tmp->mount->root!=NULL) tmp = tmp->mount->root;
        CurrWorkDir = tmp;
        memset(parse, 0, 16);
        idx = 0;
      }
    }else if(pathname[i] == 0){
      if(idx == 0){
        *target = CurrWorkDir;
        return 0;
      }
      else{
        if(CurrWorkDir->dt->type != directory) return -1;
        struct vnode* tmp;
        int errno;
        if((errno = CurrWorkDir->v_ops->lookup(CurrWorkDir,target,parse)) < 0){
          return errno;
        }
        if((*target)->dt->type == directory && (*target)->mount != NULL && (*target)->mount->root!=NULL) *target = (*target)->mount->root;
        return 0;
      }
    }else{
      parse[idx++] = pathname[i];
    }
  }
  return -2;
}

int vfs_mknod(const char* pathname, const char* device){
  int errno;
  struct vnode *tmp,*target;
  if(vfs_lookup(pathname,&tmp) >= 0){
    return -1;
  }
  int last_slash = -1;
  char path[256], component[256];
  memset(path,0,256);
  memset(component,0,256);
  for(int i=0;i<256;i++){
    if(pathname[i] == '/' && pathname[i+1]!=0 && pathname[i+1]!=' ') last_slash = i;
    if(pathname[i] == 0) break;
  }
  for(int i=0;i<last_slash;i++){
    path[i] = pathname[i];
  }
  if(last_slash == -1) {
    struct thread *t = get_current();
    tmp = t->CurWorkDir;
  }else{
    if(last_slash == 0)path[0] = '/';
    vfs_lookup(path,&tmp);
  } 
  for(int i=last_slash+1;i<256;i++){
    if(pathname[i] == 0)break;
    component[i-last_slash-1] = pathname[i];
  }
  errno = tmp->v_ops->mknod(tmp,&target,component);
  if(errno < 0) return errno;
  struct device *d = find_device(device);
  if(d == NULL) return -2;
  errno = d->setup(d,target);
  return errno;
}

long vfs_lseek(struct file* f, long offset, int whence){
  return f->f_ops->lseek64(f,offset,whence);
}

void vfs_ls(const char* pathname){
  struct vnode *tmp;
  if(pathname[0] == 0){
    vfs_lookup("/",&tmp);
  }else{
    int errno;
    if((errno = vfs_lookup(pathname,&tmp)) <0){
      uart_printf("File not found: %s\n",pathname);
      return;
    }
  }
  if(tmp->dt->type == directory){
    struct link_list* ll = tmp->dt->childs;
    while(ll != NULL){
      struct dentry* tmp2 = ll->entry;
      uart_printf("%s ",tmp2->name);
      ll = ll->next;
    }
    uart_printf("\n");
  }else{
    uart_printf("%s\n",tmp->dt->name);
  }
}

void vfs_cd(const char* pathname){
  struct thread *t = get_current();

  vfs_lookup(pathname, &(t->CurWorkDir));
}

struct vnode* allo_vnode(){
  struct vnode* vn = malloc(sizeof(struct vnode));
  delete_last_mem();
  vn->mount = NULL;
  vn->dt = allo_dentry();
  return vn;
}

struct dentry* allo_dentry(){
  struct dentry* de = malloc(sizeof(struct dentry));
  delete_last_mem();
  memset(de->name,0,32);
  return de;
}