/*
  FUSE ssd: FUSE ioctl example
  Copyright (C) 2008       SUSE Linux Products GmbH
  Copyright (C) 2008       Tejun Heo <teheo@suse.de>
  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "ssd_fuse_header.h"
#define SSD_NAME "ssd_file"

enum
{
    SSD_NONE,
    SSD_ROOT,
    SSD_FILE,
};

char _pca_state[4] = {'O', 'X', '.', '-'};

typedef enum
{
    valid,
    stale,
    empty,
} PCA_STATE;

typedef struct
{
    PCA_STATE state[PAGE_PER_BLOCK];
    int valid_count;
} PCA_INFO;

static size_t physic_size;
static size_t logic_size;
static size_t host_write_size;
static size_t nand_write_size;

typedef union pca_rule PCA_RULE;
union pca_rule
{
    unsigned int pca;
    struct
    {
        unsigned int lba : 16;
        unsigned int nand : 16;
    } fields;
};

PCA_RULE curr_pca;
static unsigned int get_next_pca();
void print_nands();
static void garbage_collection();
unsigned int *L2P, *P2L, free_block_number;
PCA_INFO *pca_info;
int empty_idx = PHYSICAL_NAND_NUM - 1;

#define PCA_TO_IDX(pca) pca.fields.nand *PAGE_PER_BLOCK + pca.fields.lba

static int ssd_resize(size_t new_size)
{
    // set logic size to new_size
    if (new_size > NAND_SIZE_KB * 1024)
    {
        return -ENOMEM;
    }
    else
    {
        logic_size = new_size;
        return 0;
    }
}

static int ssd_expand(size_t new_size)
{
    // logic must less logic limit

    if (new_size > logic_size)
    {
        return ssd_resize(new_size);
    }

    return 0;
}

static int nand_read(char *buf, int pca)
{
    char nand_name[100];
    FILE *fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    // read
    if ((fptr = fopen(nand_name, "r")))
    {
        fseek(fptr, my_pca.fields.lba * 512, SEEK_SET);
        fread(buf, 1, 512, fptr);
        fclose(fptr);
    }
    else
    {
        printf("open file fail at nand read pca = %d\n", pca);
        return -EINVAL;
    }
    return 512;
}
static int nand_write(const char *buf, int pca)
{
    char nand_name[100];
    FILE *fptr;

    PCA_RULE my_pca;
    my_pca.pca = pca;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, my_pca.fields.nand);

    // write
    if ((fptr = fopen(nand_name, "r+")))
    {
        fseek(fptr, my_pca.fields.lba * 512, SEEK_SET);
        fwrite(buf, 1, 512, fptr);
        fclose(fptr);
        physic_size++;
        pca_info[my_pca.fields.nand].valid_count++;
        pca_info[my_pca.fields.nand].state[my_pca.fields.lba] = valid;
    }
    else
    {
        printf("open file fail at nand (%s) write pca = %d, return %d\n", nand_name, pca, -EINVAL);
        return -EINVAL;
    }

    nand_write_size += 512;
    return 512;
}

static int nand_erase(int block_index)
{
    char nand_name[100];
    FILE *fptr;
    snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, block_index);
    fptr = fopen(nand_name, "w");
    if (fptr == NULL)
    {
        printf("erase nand_%d fail", block_index);
        return 0;
    }
    fclose(fptr);
    pca_info[block_index].valid_count = FREE_BLOCK;
    free_block_number++;
    return 1;
}

static void garbage_collection()
{
    int target_idx = -1, min_valid_count = PAGE_PER_BLOCK + 1;
    char *tmp_buf = calloc(512, sizeof(char));

    // find the target to erase
    for (int i = 0; i < PHYSICAL_NAND_NUM; i++)
    {
        if (pca_info[i].valid_count != FREE_BLOCK && pca_info[i].valid_count < min_valid_count)
        {
            target_idx = i;
            min_valid_count = pca_info[i].valid_count;
        }
    }

    // curr_pca has new block now
    curr_pca.fields.nand = empty_idx;
    curr_pca.fields.lba = 0;
    pca_info[empty_idx].valid_count = 0;

    for (int i = 0; i < PAGE_PER_BLOCK; i++)
    {
        if (pca_info[target_idx].state[i] != valid)
        {
            continue;
        }

        PCA_RULE old_pca;
        old_pca.fields.nand = target_idx;
        old_pca.fields.lba = i;

        nand_read(tmp_buf, old_pca.pca);
        nand_write(tmp_buf, curr_pca.pca);

        unsigned int lba = P2L[PCA_TO_IDX(old_pca)];
        L2P[lba] = curr_pca.pca;
        P2L[PCA_TO_IDX(curr_pca)] = lba;
        P2L[PCA_TO_IDX(old_pca)] = INVALID_LBA;

        curr_pca.fields.lba++;
    }

    // empty_idx will always be empty
    empty_idx = target_idx;
    nand_erase(empty_idx);

    free(tmp_buf);
    return;
}

static unsigned int get_next_block()
{
    for (int i = 0; i < PHYSICAL_NAND_NUM; i++)
    {
        if (pca_info[(curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM].valid_count == FREE_BLOCK)
        {
            curr_pca.fields.nand = (curr_pca.fields.nand + i) % PHYSICAL_NAND_NUM;
            curr_pca.fields.lba = 0;
            free_block_number--;
            pca_info[curr_pca.fields.nand].valid_count = 0;
            return curr_pca.pca;
        }
    }

    printf("[get_next_block] OUT_OF_BLOCK \n");
    return OUT_OF_BLOCK;
}
static unsigned int get_next_pca()
{
    if (curr_pca.pca == INVALID_PCA)
    {
        // init
        curr_pca.pca = 0;
        pca_info[0].valid_count = 0;
        free_block_number--;
        return curr_pca.pca;
    }

    if (curr_pca.fields.lba == 9)
    {
        if (free_block_number <= 2)
        {
            garbage_collection();
            free_block_number--;
            return curr_pca.pca;
        }
        int temp = get_next_block();
        if (temp == OUT_OF_BLOCK)
        {
            return OUT_OF_BLOCK;
        }
        else if (temp == -EINVAL)
        {
            return -EINVAL;
        }
        else
        {
            return temp;
        }
    }
    else
    {
        curr_pca.fields.lba += 1;
    }
    return curr_pca.pca;
}

static int ftl_read(char *buf, size_t lba)
{
    return nand_read(buf, L2P[lba]);
}

static int set_stale(PCA_RULE my_pca)
{
    PCA_STATE *target = &(pca_info[my_pca.fields.nand].state[my_pca.fields.lba]);
    if (my_pca.pca != INVALID_PCA && *target == valid)
    {
        *target = stale;
        pca_info[my_pca.fields.nand].valid_count--;
        return 0;
    }
}
static int set_valid(PCA_RULE my_pca)
{
    PCA_STATE *target = &(pca_info[my_pca.fields.nand].state[my_pca.fields.lba]);
    *target = valid;
    return 0;
}

static int ftl_write(const char *buf, size_t lba_range, size_t lba)
{
    PCA_RULE my_pca;
    my_pca.pca = L2P[lba];
    set_stale(my_pca);

    my_pca.pca = get_next_pca();

    L2P[lba] = my_pca.pca;
    P2L[PCA_TO_IDX(my_pca)] = lba;
    return nand_write(buf, my_pca.pca);
}

static int ssd_file_type(const char *path)
{
    if (strcmp(path, "/") == 0)
    {
        return SSD_ROOT;
    }
    if (strcmp(path, "/" SSD_NAME) == 0)
    {
        return SSD_FILE;
    }
    return SSD_NONE;
}
static int ssd_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi)
{
    (void)fi;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = time(NULL);
    switch (ssd_file_type(path))
    {
    case SSD_ROOT:
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        break;
    case SSD_FILE:
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = logic_size;
        break;
    case SSD_NONE:
        return -ENOENT;
    }
    return 0;
}
static int ssd_open(const char *path, struct fuse_file_info *fi)
{
    (void)fi;
    if (ssd_file_type(path) != SSD_NONE)
    {
        return 0;
    }
    return -ENOENT;
}
static int ssd_do_read(char *buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, rst;
    char *tmp_buf;

    // off limit
    if ((offset) >= logic_size)
    {
        return 0;
    }
    if (size > logic_size - offset)
    {
        // is valid data section
        size = logic_size - offset;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;
    tmp_buf = calloc(tmp_lba_range * 512, sizeof(char));

    for (int i = 0; i < tmp_lba_range; i++)
    {
        ftl_read(tmp_buf + i * 512, tmp_lba + i);
    }

    memcpy(buf, tmp_buf + offset % 512, size);

    free(tmp_buf);
    return size;
}
static int ssd_read(const char *path, char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi)
{
    (void)fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_read(buf, size, offset);
}
static int ssd_do_write(const char *buf, size_t size, off_t offset)
{
    int tmp_lba, tmp_lba_range, process_size;
    int idx, curr_size, remain_size, rst;
    char *tmp_buf;
    int write_start = 0, write_end = 0, buf_idx = 0;

    host_write_size += size;
    if (ssd_expand(offset + size) != 0)
    {
        return -ENOMEM;
    }

    tmp_lba = offset / 512;
    tmp_lba_range = (offset + size - 1) / 512 - (tmp_lba) + 1;

    process_size = 0;
    remain_size = size;
    curr_size = 0;
    tmp_buf = calloc(512, sizeof(char));
    for (idx = 0; idx < tmp_lba_range; idx++)
    {
        // may not align
        if (idx == 0 || idx == tmp_lba_range - 1)
        {
            nand_read(tmp_buf, L2P[tmp_lba + idx]);
            write_start = (idx == 0) ? offset % 512 : 0;
            write_end = (idx == tmp_lba_range - 1) ? (offset + size) - (tmp_lba + idx) * 512 : 512;
            for (int i = write_start; i < write_end; i++)
            {
                tmp_buf[i] = buf[buf_idx++];
            }
            ftl_write(tmp_buf, tmp_lba_range, tmp_lba + idx); 
        }
        else
        {
            ftl_write(buf + buf_idx, tmp_lba_range, tmp_lba + idx); 
            buf_idx += 512;
        }
    }
    free(tmp_buf);
    return size;
}
static int ssd_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{

    (void)fi;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    return ssd_do_write(buf, size, offset);
}
static int ssd_truncate(const char *path, off_t size,
                        struct fuse_file_info *fi)
{
    (void)fi;
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);
    memset(pca_info, FREE_BLOCK, PHYSICAL_NAND_NUM * sizeof(PCA_INFO));
    curr_pca.pca = INVALID_PCA;
    free_block_number = PHYSICAL_NAND_NUM;
    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }

    return ssd_resize(size);
}
static int ssd_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags)
{
    (void)fi;
    (void)offset;
    (void)flags;
    if (ssd_file_type(path) != SSD_ROOT)
    {
        return -ENOENT;
    }
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, SSD_NAME, NULL, 0, 0);
    return 0;
}
static int ssd_ioctl(const char *path, unsigned int cmd, void *arg,
                     struct fuse_file_info *fi, unsigned int flags, void *data)
{

    if (ssd_file_type(path) != SSD_FILE)
    {
        return -EINVAL;
    }
    if (flags & FUSE_IOCTL_COMPAT)
    {
        return -ENOSYS;
    }
    switch (cmd)
    {
    case SSD_GET_LOGIC_SIZE:
        *(size_t *)data = logic_size;
        return 0;
    case SSD_GET_PHYSIC_SIZE:
        *(size_t *)data = physic_size;
        return 0;
    case SSD_GET_WA:
        print_nands();
        *(double *)data = (double)nand_write_size / (double)host_write_size;
        return 0;
    }
    return -EINVAL;
}
static const struct fuse_operations ssd_oper =
    {
        .getattr = ssd_getattr,
        .readdir = ssd_readdir,
        .truncate = ssd_truncate,
        .open = ssd_open,
        .read = ssd_read,
        .write = ssd_write,
        .ioctl = ssd_ioctl,
};
int main(int argc, char *argv[])
{
    int idx;
    char nand_name[100];
    physic_size = 0;
    logic_size = 0;
    curr_pca.pca = INVALID_PCA;
    free_block_number = PHYSICAL_NAND_NUM;

    L2P = malloc(LOGICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(L2P, INVALID_PCA, sizeof(int) * LOGICAL_NAND_NUM * PAGE_PER_BLOCK);
    P2L = malloc(PHYSICAL_NAND_NUM * PAGE_PER_BLOCK * sizeof(int));
    memset(P2L, INVALID_LBA, sizeof(int) * PHYSICAL_NAND_NUM * PAGE_PER_BLOCK);

    pca_info = malloc(PHYSICAL_NAND_NUM * sizeof(PCA_INFO));
    memset(pca_info, FREE_BLOCK, PHYSICAL_NAND_NUM * sizeof(PCA_INFO));

    // create nand file
    for (idx = 0; idx < PHYSICAL_NAND_NUM; idx++)
    {
        FILE *fptr;
        snprintf(nand_name, 100, "%s/nand_%d", NAND_LOCATION, idx);
        fptr = fopen(nand_name, "w");
        if (fptr == NULL)
        {
            printf("open fail");
        }
        fclose(fptr);
    }
    return fuse_main(argc, argv, &ssd_oper, NULL);
}

void print_nands(void)
{
    printf("=====================print_nands======================================\n");
    printf("nand_write_size=%d, host_write_size=%d\n",nand_write_size ,host_write_size);

    for (int i = 0; i < PHYSICAL_NAND_NUM; ++i)
    {
        if (pca_info[i].valid_count == FREE_BLOCK)
        {
            printf("NAND_%d     | FREE_BLOCK\n", i);
        }
        else
        {
            printf("NAND_%d     | valid_count: %d | state: ", i, pca_info[i].valid_count);
            for (int j = 0; j < PAGE_PER_BLOCK; j++)
            {
                int idx = pca_info[i].state[j] == -1 ? 3 : pca_info[i].state[j];
                printf("%c", _pca_state[idx]);
            }
            printf("\n");
        }
    }
}