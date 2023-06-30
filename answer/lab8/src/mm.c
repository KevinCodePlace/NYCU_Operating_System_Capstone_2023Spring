#include "mm.h"
#include "mini_uart.h"
#include "list.h"
#include "utils_c.h"
#include "_cpio.h"
#include "dtb.h"
#include "exception_c.h"
#include "mmu.h"

// #define DEBUG

#define FRAME_BASE ((uintptr_t)PHYS_OFFSET + 0x0)
// get from mailbox's arm memory
#define FRAME_END ((uintptr_t)PHYS_OFFSET + 0x3b400000)

#define PAGE_SIZE 0x1000 // 4KB
#define FRAME_BINS_SIZE 12
#define MAX_ORDER (FRAME_BINS_SIZE - 1)
#define FRAME_MAX_SIZE ORDER2SIZE(MAX_ORDER)
#define ORDER2SIZE(order) (PAGE_SIZE * (1 << (order)))

#define FRAME_ARRAY_SIZE ((FRAME_END - FRAME_BASE) / PAGE_SIZE)

#define CHUNK_MIN_SIZE 32
#define CHUNK_BINS 7
#define CHUNK_MAX_ORDER (CHUNK_BINS - 1)

#define FRAME_FREE 0x8
#define FRAME_INUSE 0x4
#define FRAME_MEM_CHUNK 0x2
#define IS_INUSE(frames) (frames.flag & FRAME_INUSE)
#define IS_MEM_CHUNK(frames) (frames.flag & FRAME_MEM_CHUNK)

// for mm_int
extern char _skernel, _ekernel;
extern void *_dtb_ptr;

FrameFlag *frames;
list frame_bins[FRAME_BINS_SIZE];
Chunk *chunk_bins[CHUNK_BINS];

unsigned long *smalloc_cur = (unsigned long *)STARTUP_MEM_START;

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          utils                                             //
////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned align_up_exp(unsigned n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}
static int addr2idx(void *addr)
{
    return (((uintptr_t)addr & -PAGE_SIZE) - FRAME_BASE) / PAGE_SIZE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          startUp                                           //
////////////////////////////////////////////////////////////////////////////////////////////////

void *smalloc(size_t size)
{
    align(&size, 4); // allocated the memory size is mutiple of 4 byte;
    unsigned long *smalloc_ret = smalloc_cur;
    if ((uint64_t)smalloc_cur + size > (uint64_t)STARTUP_MEM_END)
    {
        uart_printf("[!] No enough space!\r\n");
        return NULL;
    }
    smalloc_cur += (unsigned int)size;
    return smalloc_ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          others                                             //
////////////////////////////////////////////////////////////////////////////////////////////////

void mm_init()
{
    init_buddy();
    memory_reserve((uintptr_t)pa2va(0), (uintptr_t)pa2va(0x1000)); // Spin tables for multicore boot 

    memory_reserve((uintptr_t)IDENTITY_TT_L0_VA, (uintptr_t)IDENTITY_TT_L0_VA + PAGE_SIZE); // PGD's page frame 
    memory_reserve((uintptr_t)IDENTITY_TT_L1_VA, (uintptr_t)IDENTITY_TT_L1_VA + PAGE_SIZE); // PUD's page frame 

    memory_reserve((uintptr_t)&_skernel, (uintptr_t)&_ekernel); // Kernel image  

    fdt_traverse(get_initramfs_addr);
    memory_reserve((uintptr_t)initramfs_start, (uintptr_t)initramfs_end); // Initramfs  

    memory_reserve((uintptr_t)STARTUP_MEM_START, (uintptr_t)STARTUP_MEM_END); // Simple allocator 

    memory_reserve((uintptr_t)dtb_start, (uintptr_t)dtb_end); // Devicetree 

    merge_useful_pages();
}

void memory_reserve(uintptr_t start, uintptr_t end)
{

    start = start & ~(PAGE_SIZE - 1);
    end = align_up(end - PHYS_OFFSET, PAGE_SIZE) + PHYS_OFFSET;
    for (uintptr_t i = start; i < end; i += PAGE_SIZE)
    {
        int idx = addr2idx((void *)i);
        frames[idx].flag |= FRAME_INUSE;
        frames[idx].order = 0;
    }
#ifdef DEBUG
    uart_printf("reserve addr from %x ~ %x\n", (unsigned long)start, (unsigned long)end);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                   buddy system                                             //
////////////////////////////////////////////////////////////////////////////////////////////////

static int pages_to_frame_order(unsigned pages)
{
    pages = align_up_exp(pages);
    return __builtin_ctz(pages);
}

void init_buddy()
{
    frames = (FrameFlag *)smalloc(sizeof(FrameFlag) * FRAME_ARRAY_SIZE);
    for (int i = 0; i < FRAME_BINS_SIZE; i++)
    {
        list_init(&frame_bins[i]);
    }
    for (int i = 0; i < FRAME_ARRAY_SIZE; i++)
    {
        frames[i].flag = 0;
        frames[i].order = 0;
    }
}

void merge_useful_pages()
{
    int page_idx = 0;
    list *page_addr = (list *)(FRAME_BASE);
    {   //  only insert the 4KB page to frame_bins
        while (1)
        {
            if (!IS_INUSE(frames[page_idx]))
            {
                insert_tail(&frame_bins[0], page_addr);
            }
            page_idx++;
            if (page_idx >= FRAME_ARRAY_SIZE)
            {
                break;
            }
            page_addr = (void *)(FRAME_BASE + page_idx * PAGE_SIZE);
        }
    }

    for (int order = 0; order < MAX_ORDER; order++)
    {   //  merging the pages by left page
        page_idx = 0;
        page_addr = (list *)(FRAME_BASE);
        int buddy_page_idx = 0;
        list *buddy_addr = (list *)(FRAME_BASE);
        while (1)
        {
            buddy_page_idx = page_idx ^ (1 << order);
            buddy_addr = (void *)(FRAME_BASE + buddy_page_idx * PAGE_SIZE);

            if (!IS_INUSE(frames[page_idx]) &&
                frames[page_idx].order == order &&
                buddy_page_idx < FRAME_ARRAY_SIZE &&
                !IS_INUSE(frames[buddy_page_idx]) &&
                frames[buddy_page_idx].order == order)
            {
                unlink((list *)page_addr);
                unlink((list *)buddy_addr);
                insert_tail(&frame_bins[order + 1], page_addr);
                frames[page_idx].order = order + 1;
            }
            page_idx += (1 << (order + 1));
            if (page_idx >= FRAME_ARRAY_SIZE)
            {
                break;
            }
            page_addr = (void *)(FRAME_BASE + page_idx * PAGE_SIZE);
        }
    }
}

static void *split_frames(int order, int target_order)
{
    list *ptr = remove_head(&frame_bins[order]);

#ifdef DEBUG
    uart_printf("split frame: %x\n", (unsigned long)ptr);
#endif

    for (int i = order; i > target_order; i--)
    { /* insert splitted frame to bin list */
        list *half_right = (list *)((char *)ptr + ORDER2SIZE(i - 1));

        insert_head(&frame_bins[i - 1], half_right);
        frames[((uintptr_t)half_right - FRAME_BASE) / PAGE_SIZE].order = i - 1;

#ifdef DEBUG
        uart_printf("insert frame at %x\n", (unsigned long)half_right);
#endif
    }
    int idx = addr2idx(ptr);
    frames[idx].order = target_order;
    frames[idx].flag |= FRAME_INUSE;
    return ptr;
}

void *alloc_pages(unsigned int pages)
{
    int target_order = pages_to_frame_order(pages);
    if (frame_bins[target_order].next != &frame_bins[target_order])
    {
        list *ptr = remove_head(&frame_bins[target_order]);
#ifdef DEBUG
        uart_printf("return page at: %x\n", (unsigned long)ptr);
#endif
        int idx = addr2idx(ptr);
        frames[idx].order = target_order;
        frames[idx].flag |= FRAME_INUSE;
        return ptr;
    }
    else
    {
        for (int i = target_order; i < FRAME_BINS_SIZE; i++)
        {
            if (frame_bins[i].next != &frame_bins[i])
            {
                return split_frames(i, target_order);
            }
        }
    }

    uart_send_string("alloc page return NULL");
    return NULL;
}

void free_pages(void *victim)
{
    int page_idx = ((uintptr_t)victim - FRAME_BASE) / PAGE_SIZE;
    if (!IS_INUSE(frames[page_idx]))
    {
        uart_printf("Error! double free the memory at %x\n", (uintptr_t)victim);
        return;
    }
    unsigned int order = frames[page_idx].order;
    int buddy_page_idx = page_idx ^ (1 << order);
    frames[page_idx].flag &= ~FRAME_INUSE;

    while (order <= MAX_ORDER &&
           !IS_INUSE(frames[buddy_page_idx]) &&
           order == frames[buddy_page_idx].order)
    {
        void *buddy_victim = (void *)(FRAME_BASE + buddy_page_idx * PAGE_SIZE);
        unlink((list *)buddy_victim);

#ifdef DEBUG
        uart_printf("merge buddy frame: %x \n", (unsigned long)buddy_victim);
#endif
        order += 1;
        victim = page_idx < buddy_page_idx ? victim : buddy_victim;
        page_idx = page_idx < buddy_page_idx ? page_idx : buddy_page_idx;
        buddy_page_idx = page_idx ^ (1 << order);
    }

    insert_head(&frame_bins[order], victim);
    frames[page_idx].order = order;

#ifdef DEBUG
    uart_printf("attach frame: %x \n\n", (unsigned long)victim);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          chunks                                            //
////////////////////////////////////////////////////////////////////////////////////////////////

static int size_to_chunk_order(unsigned int size)
{
    size = align_up_exp(size);
    size /= CHUNK_MIN_SIZE;
    return __builtin_ctz(size);
}

static void *get_chunk(uint32_t size)
{
    int order = size_to_chunk_order(size);

    void *ptr = chunk_bins[order];
    if (ptr)
    {
        chunk_bins[order] = chunk_bins[order]->next;
        int idx = addr2idx(ptr);
        frames[idx].ref_count += 1;

#ifdef DEBUG
        uart_printf("detach chunk at %x\n", (unsigned long)ptr);
#endif
    }

    return ptr;
}

static void alloc_chunk(void *mem, int size)
{
    int count = PAGE_SIZE / size;
    int idx = addr2idx(mem);
    int order = size_to_chunk_order(size);
    frames[idx].flag |= FRAME_MEM_CHUNK;
    frames[idx].ref_count = 0;
    frames[idx].chunk_order = order;
    for (int i = 0; i < count; i++)
    {
        Chunk *ptr = (Chunk *)((uintptr_t)mem + i * size);
        ptr->next = chunk_bins[order];
        chunk_bins[order] = ptr;

#ifdef DEBUG
        uart_printf("insert chunk at %x\n", (unsigned long)ptr);
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                           dynamic memory simple_allocator                                         //
////////////////////////////////////////////////////////////////////////////////////////////////

void *kmalloc(unsigned int size)
{
    unsigned int aligned_page_size = align_up(size, PAGE_SIZE);
    if (aligned_page_size > FRAME_MAX_SIZE)
    {
        return NULL;
    }

    size = size < CHUNK_MIN_SIZE ? CHUNK_MIN_SIZE : size;

    size_t flag = disable_irq();

    void *ptr;
    if (align_up_exp(size) < PAGE_SIZE) // just allocate a small chunk
    {
        size = align_up_exp(size);
        ptr = get_chunk(size);

        if (!ptr)
        {
            void *mem = alloc_pages(1);
            alloc_chunk(mem, size);
            ptr = get_chunk(size);
        }
    }
    else
    {
        unsigned int pages = aligned_page_size / PAGE_SIZE;
        ptr = alloc_pages(pages);
    }
    irq_restore(flag);
    return ptr;
}
void *kcalloc(unsigned int size)
{
    void *p = kmalloc(size);
    if (!p)
    {
        return NULL;
    }
    if (size < PAGE_SIZE)
    {
        size = align_up_exp(size);
    }
    else
    {
        size = align_up(size, PAGE_SIZE);
    }
    memset(p, 0, size);

    return p;
}

void kfree(void *ptr)
{
    int idx = addr2idx(ptr);
    if (idx >= FRAME_ARRAY_SIZE)
    {
        uart_send_string("Error! kfree wrong address\n");
        return;
    }
    size_t flag = disable_irq();
    if (IS_MEM_CHUNK(frames[idx]))
    {
        int order = frames[idx].chunk_order;
        ((Chunk *)ptr)->next = chunk_bins[order];
        chunk_bins[order] = ptr;
        frames[idx].ref_count -= 1;

#ifdef DEBUG
        uart_printf("free chunk at %x\n", (unsigned long)ptr);
#endif
    }
    else
    {
        free_pages(ptr);
    }
    irq_restore(flag);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          test                                              //
////////////////////////////////////////////////////////////////////////////////////////////////

void test_buddy()
{
    int test_size = 5;
    void *a[test_size];
    uart_send_string("\n\n-----  Malloc  -----\n");
    for (int i = 0; i < test_size; i++)
    {
        a[i] = alloc_pages(test_size);
    }
    uart_send_string("\n\n-----  Free  -----\n");
    for (int i = 0; i < test_size; i++)
    {
        free_pages(a[i]);
    }
}

struct test_b
{
    double b1, b2, b3, b4, b5, b6;
};

void test_dynamic_alloc()
{
    uart_send_string("allocate a1\n");
    int *a1 = kmalloc(sizeof(int));
    uart_send_string("allocate a2\n");
    int *a2 = kmalloc(sizeof(int));
    uart_send_string("allocate b\n");
    struct test_b *b = kmalloc(sizeof(struct test_b));

    uart_send_string("free a1\n");
    kfree(a1);
    uart_send_string("free b\n");
    kfree(b);
    uart_send_string("free a2\n");
    kfree(a2);
}
