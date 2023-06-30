#ifndef __TMPFS_H_
#define __TMPFS_H_

#define COMPONENT_SIZE 16
struct filesystem *tmpfs_create();

extern struct file_operations tmpfs_f_ops;
extern struct vnode_operations tmpfs_v_ops;
#endif