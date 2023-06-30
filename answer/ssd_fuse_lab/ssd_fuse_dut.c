/*
  FUSE ssdlient: FUSE ioctl example client
  Copyright (C) 2008       SUSE Linux Products GmbH
  Copyright (C) 2008       Tejun Heo <teheo@suse.de>
  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include "ssd_fuse_header.h"
#include <time.h>
const char* usage =
    "Usage: ssd_fuse SSD_FILE COMMAND\n"
    "\n"
    "COMMANDS\n"
    "  l    : get logic size \n"
    "  p    : get physical size \n"
    "  r SIZE [OFF] : read SIZE bytes @ OFF (dfl 0) and output to stdout\n"
    "  w SIZE [OFF] : write SIZE bytes @ OFF (dfl 0) from random\n"
    "  W    : write amplification factor\n"
    "\n";
static int do_rw(FILE* fd, int is_read, size_t size, off_t offset)
{
    char* buf;
    int idx;
    ssize_t ret;
    buf = calloc(1, size);

    if (!buf)
    {
        fprintf(stderr, "failed to allocated %zu bytes\n", size);
        return -1;
    }
    if (is_read)
    {
        printf("dut do read size %ld, off %d\n", size, (int)offset);
        fseek( fd, offset, SEEK_SET );
        ret = fread( buf, 1, size, fd);
        if (ret >= 0)
        {
            fwrite(buf, 1, ret, stdout);
        }
    }
    else
    {
        for ( idx = 0; idx < size; idx++)
        {
            buf[idx] = idx;
        }
        printf("dut do write size %ld, off %d\n", size, (int)offset);
        fseek( fd, offset, SEEK_SET );
        printf("fseek \n");
        ret = fwrite(buf, 1, size, fd);
        //arg.size = fread(arg.buf, 1, size, stdin);
        fprintf(stderr, "Writing %zu bytes\n", size);
    }
    if (ret < 0)
    {
        perror("ioctl");
    }

    free(buf);
    return ret;
}
int main(int argc, char** argv)
{
    size_t param[2] = { };
    size_t size;
    char cmd;
    char* path;
    FILE* fptr;
    int fd, i, rc;

    if (argc < 3)
    {
        goto usage;
    }
    path = argv[1];
    cmd = tolower(argv[2][0]);
    cmd = argv[2][0];
    argc -= 3;
    argv += 3;
    for (i = 0; i < argc; i++)
    {
        char* endp;
        param[i] = strtoul(argv[i], &endp, 0);
        if (endp == argv[i] || *endp != '\0')
        {
            goto usage;
        }
    }
    switch (cmd)
    {
        case 'l':
            fd = open(path, O_RDWR);
            if (fd < 0)
            {
                perror("open");
                return 1;
            }
            if (ioctl(fd, SSD_GET_LOGIC_SIZE, &size))
            {
                perror("ioctl");
                goto error;
            }
            printf("%zu\n", size);
            close(fd);
            return 0;
        case 'p':
            fd = open(path, O_RDWR);
            if (fd < 0)
            {
                perror("open");
                return 1;
            }
            if (ioctl(fd, SSD_GET_PHYSIC_SIZE, &size))
            {
                perror("ioctl");
                goto error;
            }
            printf("%zu\n", size);
            close(fd);
            return 0;
        case 'r':
        case 'w':
            if ( !(fptr = fopen(path, "r+")))
            {
                perror("open");
                return 1;
            }
            rc = do_rw(fptr, cmd == 'r', param[0], param[1]);
            if (rc < 0)
            {
                goto error;
            }
            fprintf(stderr, "transferred %d bytes \n", rc);

            fclose(fptr);
            return 0;
        case 'W':
            fd = open(path, O_RDWR);
            if (fd < 0)
            {
                perror("open");
                return 1;
            }
            double wa;
            if (ioctl(fd, SSD_GET_WA, &wa))
            {
                perror("ioctl");
                goto error;
            }
            printf("%f\n", wa);
            close(fd);
            return 0;
    }
usage:
    fprintf(stderr, "%s", usage);
    return 1;
error:

    return 1;
}