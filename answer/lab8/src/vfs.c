#include "vfs.h"
#include "utils_c.h"
#include "mm.h"
#include "tmpfs.h"
#include "initramfs.h"
#include "mini_uart.h"
#include "current.h"
#include "sd.h"
#include "fat32.h"

list fs_list = LIST_HEAD_INIT(fs_list);
struct mount *rootfs, *initramfs, *fat32;

static const char *next_lvl_path(const char *src, char *dst, int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (src[i] == 0)
        {
            dst[i] = 0;
            return 0;
        }
        else if (src[i] == '/')
        {
            dst[i] = 0;
            return src + i + 1;
        }
        else
            dst[i] = src[i];
    }
    return 0;
}

char *get_path(struct vnode **node)
{
    /* return a absolute path for node */
    int count = 0;
    struct vnode *itr = *node;
    char *compenent[128]; // brute force
    for (int i = 0; i < 128; i++)
    {
        compenent[i] = NULL;
    }
    while (itr->parent != itr)
    {
        int strlen = utils_strlen(itr->name);
        compenent[count] = kcalloc(sizeof(char) * strlen);
        memcpy(compenent[count], itr->name, strlen);
        count++;
        itr = itr->parent;
    }
    count -= 1;

    char *ret = kcalloc((count + 1) * COMPONENT_SIZE * sizeof(char)); // +1 for slash
    int index = 1;

    ret[0] = '/';

    for (int i = count - 1; i >= 0; i--)
    {
        for (int j = 0; j < utils_strlen(compenent[i]) - 1; j++)
        {
            ret[index++] = compenent[i][j];
        }
        ret[index++] = '/';
    }
    ret[index - 1] = '\0';
    for (int i = count - 1; i >= 0; i--)
    {
        kfree(compenent[i]);
    }

    return ret;
}

const char *path_to_abs(const char *pathname)
{
    struct vnode *target_node = NULL;
    const char *_pathname = pathname;

    if (pathname[0] == '/')
    {
        return pathname;
    }
    else if (pathname[0] == '.')
    {
        if (pathname[1] == '.')
        {
            target_node = current->pwd->parent;
            _pathname = &pathname[3];
        }
        else
        {
            target_node = current->pwd;
            _pathname = &pathname[2];
        }
    }
    else
    {
        target_node = current->pwd;
        _pathname = pathname;
    }

    char *prefix = get_path(&target_node);
    int prefix_len = utils_strlen(prefix) - 1;
    int _pathname_len = utils_strlen(_pathname);

    char *ret = kcalloc((prefix_len + _pathname_len) * sizeof(char));
    memcpy(ret, prefix, prefix_len);
    ret[prefix_len] = '/';
    memcpy(ret + prefix_len + 1, _pathname, _pathname_len);
    ret[prefix_len + _pathname_len] = '\0';

    return ret;
}

int set_itr_node(const char *pathname, struct vnode **vnode)
{
    /*set the beginning of traversing and return the index of _pathname start*/
    struct vnode *target_node = NULL;
    int index = 0;

    if (pathname[0] == '/')
    {
        *vnode = rootfs->root;
        index = 1;
        return index;
    }
    else if (pathname[0] == '.')
    {
        if (pathname[1] == '.')
        {
            target_node = current->pwd->parent;
            index = 3;
        }
        else
        {
            target_node = current->pwd;
            index = 2;
        }
    }
    else
    {
        target_node = current->pwd;
    }

    *vnode = target_node;
    return index;
}

int set_couple_node(struct vnode **parent, struct vnode **child, const char *pathname, char *prefix)
{
    /* if the pathname='/dir1/dir2/text' -> parent=dir2  child=text */
    struct vnode *target_node = NULL;

    struct vnode *itr = NULL;
    int index = set_itr_node(pathname, &itr);
    const char *_pathname = &pathname[index]; // pathname=../dir2/file   _pathname=dir2/file;

    /* if pathname='/' -> _pathname='' */
    if (!_pathname[0])
    {
        *parent = rootfs->root;
        *child = rootfs->root;
        next_lvl_path(_pathname, prefix, COMPONENT_SIZE); // update prefix
        return 0;
    }

    /* The itr will be a directory (root is the toppest) */
    while (1)
    {
        _pathname = next_lvl_path(_pathname, prefix, COMPONENT_SIZE);
        if (itr->v_ops->lookup(itr, &target_node, prefix) == -1)
        {
            *parent = itr;
            *child = NULL;
            return -1;
        }
        else
        {
            if (S_ISDIR(target_node->f_mode))
            {
                if (!_pathname)
                {
                    /* encounter the last component */
                    *child = target_node;
                    *parent = target_node->parent;
                    return 0;
                }
                itr = target_node;
            }
            else if (S_ISREG(target_node->f_mode))
            {
                *child = target_node;
                *parent = target_node->parent;
                return 0;
            }
        }
    }

    return 0;
}

struct vnode *vnode_create(const char *name, unsigned int flags)
{
    struct vnode *vnode = kcalloc(sizeof(struct vnode));

    list_init(&vnode->childs);
    list_init(&vnode->self);
    vnode->child_num = 0;
    vnode->parent = NULL;

    size_t name_len = utils_strlen(name);
    vnode->name = kcalloc(sizeof(name_len));
    memcpy(vnode->name, name, name_len);

    vnode->f_mode = flags;

    vnode->content = NULL;
    vnode->content_size = 0;

    vnode->internal = NULL;

    return vnode;
}

void init_rootfs()
{
    if (fs_register(tmpfs_create()))
    {
        uart_send_string("[fs] Error! fail to register tmpfs\n");
    }
    rootfs = kcalloc(sizeof(struct mount));
    struct filesystem *fs = fs_get("tmpfs");
    if (!fs)
    {
        uart_send_string("[fs] Error! fail to get tmpfs\n");
        return;
    }
    rootfs->fs = fs;
    rootfs->root = vnode_create("", S_IFDIR);
    rootfs->fs->setup_mount(rootfs->fs, rootfs);
#ifdef FS_DEBUG
    uart_send_string("[fs] init rootfs success\n");
#endif
}
void init_initramfs()
{
    struct vnode *initramfs_root = NULL;
    if (vfs_lookup("/initramfs", &initramfs_root))
    {
        uart_send_string("[init_initramfs] fail to lookup /initramfs\n");
    }
    if (fs_register(initramfs_create()))
    {
        uart_send_string("[fs] Error! fail to register tmpfs\n");
    }
    initramfs = kcalloc(sizeof(struct mount));
    struct filesystem *fs = fs_get("initramfs");
    if (!fs)
    {
        uart_send_string("[fs] Error! fail to get initramfs\n");
        return;
    }
    initramfs->fs = fs;
    initramfs->root = initramfs_root;
    initramfs->fs->setup_mount(initramfs->fs, initramfs);
#ifdef FS_DEBUG
    uart_send_string("[fs] init initramfs success\n");
#endif
}
void init_fat32()
{
    struct vnode *fat32_root = NULL;
    if (vfs_lookup("/boot", &fat32_root))
    {
        uart_send_string("[init_fat32] fail to lookup /boot\n");
    }
    if (fs_register(fat32_create()))
    {
        uart_send_string("[init_fat32] Error! fail to register fat32\n");
    }
    fat32 = kcalloc(sizeof(struct mount));
    struct filesystem *fs = fs_get("fat32");
    if (!fs)
    {
        uart_send_string("[fs] Error! fail to get fat32\n");
        return;
    }
    fat32->fs = fs;
    fat32->root = fat32_root;
    fat32->fs->setup_mount(fat32->fs, fat32);
#ifdef FS_DEBUG
    uart_send_string("[fs] init fat32 success\n");
#endif
}

void fs_init()
{
    sd_init();
    init_rootfs();
    vfs_mkdir("/initramfs");
    init_initramfs();
    vfs_mkdir("/boot");
    init_fat32();
}

int fs_register(struct filesystem *fs)
{
    if (!fs_get(fs->name))
    {
        insert_tail(&fs_list, &fs->list);
        return 0;
    }
    return -1;
}

struct filesystem *fs_get(const char *name)
{
    struct filesystem *fs;
    list_for_each_entry(fs, &fs_list, list)
    {
        if (!utils_str_compare(fs->name, name))
        {
            return fs;
        }
    }
    return NULL;
}

int vfs_open(const char *pathname, int flags, struct file **target)
{
    int res = 0;
    struct vnode *target_node = NULL;
    res = vfs_lookup(pathname, &target_node);

    if (res == -1 && !(flags & O_CREAT))
    {
        /* can't lookup and without O_CREAT flag */
        uart_printf("[vfs_open] fail to open the %s\n", pathname);
        return -1;
    }

    *target = kcalloc(sizeof(struct file));
    (*target)->flags = flags;
    (*target)->f_ops = target_node->f_ops;
    (*target)->f_pos = 0;

    if (!res)
    {
        (*target)->vnode = target_node;
#ifdef FS_DEBUG
        uart_printf("[vfs_open] find it by lookup\n");
#endif
        return 0;
    }

    struct vnode *parent = NULL, *child = NULL;
    char child_name[COMPONENT_SIZE];

    set_couple_node(&parent, &child, pathname, child_name);
    if (!child)
    {
        parent->v_ops->create(parent, &child, child_name);
        (*target)->vnode = child;
        return 0;
    }

    return -1;
}

int vfs_close(struct file *file)
{
    return file->vnode->f_ops->close(file);
}

int vfs_write(struct file *file, const void *buf, size_t len)
{
    return file->vnode->f_ops->write(file, buf, len);
}

int vfs_read(struct file *file, void *buf, size_t len)
{
    return file->vnode->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char *pathname)
{
    struct vnode *parent = NULL, *child = NULL;
    char child_name[COMPONENT_SIZE];

    set_couple_node(&parent, &child, pathname, child_name);

    if (child)
    {
        uart_printf("[vfs_mkdir] the %s is already exist\n", pathname);
        return -1;
    }
    parent->v_ops->mkdir(parent, &child, child_name);
    return 0;
}
int vfs_mount(const char *target, const char *filesystem)
{
    struct filesystem *fs = fs_get(filesystem);
    if (!fs)
    {
        uart_send_string("[vfs_mount] Error! fail to get fs\n");
        return -1;
    }

    struct vnode *vnode;
    if (vfs_lookup(target, &vnode) == -1)
    {
        uart_send_string("[vfs_mount] Error! fail to lookup\n");
        return -1;
    }

    if (!S_ISDIR(vnode->f_mode))
    {
        uart_send_string("[vfs_mount] Error! the target is not a directory\n");
        return -1;
    }

    struct mount *new_mount = kcalloc(sizeof(struct mount));
    new_mount->fs = fs;
    new_mount->root = vnode;
    new_mount->fs->setup_mount(new_mount->fs, new_mount);

    return 0;
}

int vfs_lookup(const char *pathname, struct vnode **target)
{
    struct vnode *parent = NULL, *child = NULL;
    char child_name[COMPONENT_SIZE];
    set_couple_node(&parent, &child, pathname, child_name);
    if (child)
    {
        *target = child;
        return 0;
    }
    return -1;
}

int vfs_chdir(const char *pathname)
{
    struct vnode *node = NULL;

    if (vfs_lookup(pathname, &node) == -1)
    {
        uart_printf("[vfs_chdir] fail to loopup the dir:%s\n", pathname);
        return -1;
    }
    current->pwd = node;
    return 0;
}

void vfs_test()
{
    // struct file *f1;

    // absolute path
    /*
    vfs_mkdir("/dir1");
    vfs_mkdir("/dir4");
    vfs_mkdir("/dir1/dir3");
    vfs_mkdir("/dir1/dir2");

    if (!vfs_open("/dir1/dir3/text", O_CREAT, &f1))
    {
        uart_send_string("[v] open the /dir1/dir3/text \n");
    }
    char buf1[10] = "012345678\n";
    char buf2[10] = {0};
    vfs_write(f1, buf1, 8);
    vfs_close(f1);

    vfs_open("/dir1/dir3/text", 0, &f1);
    vfs_read(f1, buf2, 8);
    uart_printf("read buf2 :%s\n", buf2);
    vfs_close(f1);

    if (vfs_mkdir("/dir1/dir2/dir5"))
    {
        uart_send_string("[v] vfs_mkdir /dir1/dir2/dir5 fail \n");
    }
    if (!vfs_mount("/dir1/dir2/dir5", "tmpfs"))
    {
        uart_send_string("[v] vfs_mount /dir1/dir2/dir5 success \n");
    }

    vfs_open("/dir4/t2", O_CREAT, &f1);
    vfs_close(f1);

    vfs_mkdir("/dir1/dir1");
    vfs_open("/dir1/dir1/t3", O_CREAT, &f1);
    vfs_close(f1);

    vfs_chdir("/dir1");
    */

    // relative path
    /*if (vfs_mkdir("/dir1") == -1)
    {
        uart_printf("fail to mkdir /dir1\n");
    }
    vfs_chdir("/dir1");
    if (vfs_mkdir("../dir4") == -1)
    {
        uart_printf("fail to mkdir ../dir4\n");
    }
    vfs_mkdir("dir2");
    vfs_mkdir("./dir3");
    vfs_open("../dir4/t2", O_CREAT, &f1);

    char buf1[10] = "012345678\n";
    char buf2[10] = {0};
    vfs_write(f1, buf1, 8);
    vfs_close(f1);
    vfs_open("../dir4/t2", 0, &f1);
    vfs_read(f1, buf2, 8);
    uart_printf("read buf2 :%s\n", buf2);
    vfs_close(f1);
    if (vfs_mkdir("./dir2/dir5"))
    {
        uart_send_string("[v] vfs_mkdir ./dir2/dir5 fail \n");
    }
    if (!vfs_mount("dir2/dir5", "tmpfs"))
    {
        uart_send_string("[v] vfs_mount dir2/dir5 success \n");
    }*/

    /*
    uart_printf("---------fat_basic 2-------\n");
    struct vnode *node = NULL;
    if (vfs_lookup("/boot/FAT_R.TXT", &node))
    {
        uart_send_string("[test] fail to lookup /boot/FAT_R.TXT\n");
    }
    vfs_open("/boot/FAT_R.TXT", O_CREAT, &f1);
    char buf1[10] = "012345678\n";
    char buf2[10] = {0};
    vfs_write(f1, buf1, 8);
    vfs_close(f1);
    vfs_open("/boot/FAT_R.TXT", 0, &f1);
    vfs_read(f1, buf2, 8);
    uart_printf("read buf2 :%s\n", buf2);
    vfs_close(f1);
    uart_printf("---------fat_basic 2-------\n");
    vfs_open("/boot/AAA.TXT", O_CREAT, &f1);
    if (vfs_open("/boot/AAA.TXT", 0, &f1))
    {
        uart_send_string("[test] fail to open /boot/AAA.TXT\n");
    }
    char buf3[10] = "876543210\n";
    char buf4[10] = {0};
    vfs_write(f1, buf3, 8);
    uart_printf("[test] close\n");
    vfs_close(f1);
    uart_printf("[test] read\n");
    vfs_open("/boot/AAA.TXT", 0, &f1);
    vfs_read(f1, buf4, 8);
    uart_printf("read buf4 :%s\n", buf4);
    */
}