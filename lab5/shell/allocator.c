#include "header/allocator.h"
#include "header/utils.h"
#include "header/uart.h"
#include "header/dtb.h"

#define SIMPLE_MALLOC_BUFFER_SIZE 8192
static unsigned char simple_malloc_buffer[SIMPLE_MALLOC_BUFFER_SIZE];
static unsigned long simple_malloc_offset = 0;
struct buddy_alloc *_global_allocator = 0;
void *(*balloc_nalloc)(uint64_t);

void* simple_malloc(unsigned long size){
	//align to 8 bytes
	utils_align(&size,8);

	if(simple_malloc_offset + size > SIMPLE_MALLOC_BUFFER_SIZE) {
		//Not enough space left
		return (void*) 0;
	}
	void* allocated = (void *)&simple_malloc_buffer[simple_malloc_offset];
	simple_malloc_offset += size;
	
	return allocated;
}

void buddy_alloc_reserve(void *start, void *end)
{
    uart_send_string("\n\nreserve start address: ");
    uart_hexll((uint64_t)start);ENDL;
	uart_send_string("reserve end address: ");
    uart_hexll((uint64_t)end);ENDL;
    uart_send_string("\r\nRESERVED\r\n");
    
    for (uint64_t i = (uint64_t)start >> 12;
         i < ((uint64_t) utils_ret_align(end, (1<<12))) >> 12;
         i++) {
        _global_allocator->entries[i] = -1;
    }
}



void buddy_alloc_preinit(struct buddy_alloc *header)
{
    uint64_t page_count = header->size / header->page_size;
    uint64_t header_size = sizeof(struct buddy_alloc);
    
    uint64_t entries_size = page_count * sizeof(buddy_alloc_entry);
    uart_send_string("Entries size: ");
    uart_hex(entries_size);ENDL;
    uint64_t freelist_size = (header->order + 1) * sizeof(uint64_t);
    header->entries = (buddy_alloc_entry *) ((uint64_t) header + header_size);
    header->freelist = (struct buddy_alloc_node **) (((uint64_t) header->entries) + entries_size);
    header->mem = 0;
    _global_allocator = header;
    for (int i = 0; i <= header->order; i++)
        header->freelist[i] = 0;
    for (uint64_t i = 0; i < page_count; i++)
        header->entries[i] = 0;
    buddy_alloc_reserve(header, header + header_size + entries_size + freelist_size);
}

void node_alloc_init(void *p, uint64_t size, uint32_t unit_size)
{
    struct node_alloc_node *cur = (struct node_alloc_node *) p;
    cur->next = 0;
    cur->size = unit_size;
    
    uint64_t full_size = sizeof(struct node_alloc_node) + unit_size;
    for (uint64_t i = full_size; i < size; i += full_size) {
        struct node_alloc_node *h = (struct node_alloc_node *)(p + i);
        h->size = unit_size;
        h->next = 0;
        cur->next = h;
        cur = h;
    }
}

void *node_alloc(struct node_alloc_node *head)
{
    struct node_alloc_node *cur = head->next;
    if (cur) {
        head->next = cur->next;
        //current start address of node + header_sz so it will be storage addr 
        return (void *) (((uint64_t) cur) + sizeof(struct node_alloc_node));
    }
}

void *internal_node_alloc(uint64_t dontcare) {
	return node_alloc(_global_allocator->nhead);
}

void nfree(struct node_alloc_node *head, void *p)
{
    struct node_alloc_node *n = (struct node_alloc_node *)(p - sizeof(struct node_alloc_node));
    n->next = head->next;
    head->next = n;
}


void buddy_alloc_init()
{
    struct buddy_alloc *h = _global_allocator;
    uint64_t page_count = h->size / h->page_size, frame = 0;
    h->order = MIN(63 - __builtin_clzll(page_count), h->order);

    // initialize a node-specialized allocator at the first page available
    // the nallacator will be hooked up with generic malloc after it is available
    struct node_alloc_node *nhead;
    {
        //frame will go to available page
        for (; h->entries[frame] < 0; frame++);
        nhead = (struct node_alloc_node *) (frame << 12);
        node_alloc_init((void *) nhead, 1 << 12, sizeof(struct buddy_alloc_node));
        balloc_nalloc = internal_node_alloc;
        h->entries[frame++] = -1;
        h->nhead = nhead;
    }
    for (; frame < page_count;) {
        // find next available page
        uart_send_string("\n\nframe occupy");
        uart_hex(frame);ENDL;

        for (; h->entries[frame] < 0; frame++);

        uart_send_string("\n\nthis is start of available frame: ");
        uart_hex(frame);ENDL;ENDL;


        // find next unavailable page
        uint64_t e = frame + 1;
        for (; h->entries[e] >= 0; e++){
          if(e==page_count)break;
        }
        uart_send_string("\n\nthis is end of avai frame: ");
        uart_hex(e);ENDL;ENDL;


        for (uint64_t cur_order = h->order; frame < e;) {
            uint64_t fcount = (1 << cur_order);
            if (frame + fcount > e) {
                cur_order--;
                continue;
            }
            h->entries[frame] = (buddy_alloc_entry) cur_order;

            // TODO: linked list
            struct buddy_alloc_node **cur = &h->freelist[cur_order];
            while (*cur)
                cur = &(*cur)->next;
            struct buddy_alloc_node *n = balloc_nalloc(sizeof(struct buddy_alloc_node));
            n->idx = frame;
            n->next = 0;
            *cur = n;

            for (uint64_t j = frame + 1; j < frame + fcount; j++)
                h->entries[j] = IS_CHILD;
            frame += fcount;
        }
        uart_send_string("I'm free!!!");
    }
}



void buddy_alloc_state(struct buddy_alloc *b)
{
    void *_p_args[2] = {0, 0};
    uint64_t size = 0;
    for (int32_t i = 0; i <= b->order; i++) {
        struct buddy_alloc_node *cur = b->freelist[i];
        uint32_t cnt = 0;
        while (cur) {
            cnt++;
            cur = cur->next;
        }
        size += (1 << i) * cnt;
        _p_args[0] = (void *) i; _p_args[1] = (void *) cnt;
		uart_send_string("ORDER ");
		uart_hex(_p_args[0]);
		uart_send_string(" has ");
		uart_hex(_p_args[1]);
		uart_send_string(" blocks available.");ENDL;
        //utils_printf("ORDER %u has %u blocks available.\r\n", _p_args);
    }
    uart_send_string("AVAILABLE PAGES: ");
    uart_hex(size);ENDL;ENDL;
}

uint32_t binsize[4] = {sizeof(struct buddy_alloc_node), 64, 128, 256};
struct node_alloc_node *bins[4];

void *buddy_alloc(uint64_t size)
{
    uint64_t pc = size / _global_allocator->page_size + ((size % _global_allocator->page_size) > 0);
    uint64_t h = 63 - __builtin_clzll(pc);
    uint64_t order = MIN(h + ((pc - (1 << h)) > 0), _global_allocator->order);
    int32_t next_lowest_avail_order = -1;
    for (uint32_t c = order; c <= _global_allocator->order; c++)
        if (_global_allocator->freelist[c]) {
            next_lowest_avail_order = (int32_t) c;
            break;
        }
    if (next_lowest_avail_order < 0)
        return NULL;

    for (uint32_t c = next_lowest_avail_order; c > order; c--) {
        struct buddy_alloc_node *n = _global_allocator->freelist[c];
        if (!n)
            break;

        _global_allocator->freelist[c] = n->next;
        struct buddy_alloc_node *n1 = balloc_nalloc(sizeof(struct buddy_alloc_node));
        struct buddy_alloc_node *n2 = balloc_nalloc(sizeof(struct buddy_alloc_node));
        n1->idx = n->idx;
        n2->idx = n1->idx + (1 << (c - 1));
        n1->next = n2;
        n2->next = _global_allocator->freelist[c - 1];
        _global_allocator->freelist[c - 1] = n1;
        _global_allocator->entries[n1->idx] = (buddy_alloc_entry) (c - 1);
        _global_allocator->entries[n2->idx] = (buddy_alloc_entry) (c - 1);
        nfree(_global_allocator->nhead, n);
    }
    struct buddy_alloc_node *n = _global_allocator->freelist[order];
    if (!n)
        return NULL;
    _global_allocator->freelist[order] = n->next;
    _global_allocator->entries[n->idx] = (buddy_alloc_entry) (((int32_t) _global_allocator->entries[n->idx] * -1) - 1);
    void *ret = (void *) (n->idx * (_global_allocator->page_size));
    nfree(_global_allocator->nhead, n);
    return ret;
}


void *buddy_malloc(uint64_t x)
{
    //i will be in [balloc_node, 64, 96, 128]
    for (int i = 0; i < sizeof(binsize)/sizeof(binsize[0]); i++) {
        //if current size of malloc is bigger than required size then use that malloc
        if (binsize[i] > x) {
            void *ret = node_alloc(bins[i]);
            if (ret)
                return ret;
            if (rejuv_node_alloc(i) < 0) {
                uart_send_string("!!!BUG!!!: CANT REJUV BINS\r\n");
                while(1);
            }
            ret = node_alloc(bins[i]);
            if (!ret) {
                uart_send_string("!!!BUG!!!: CANT REJUV BINS\r\n");
                while(1);
            }
            return ret;
        }
    }
    // small allocator can't provide, propagate to balloc
    return buddy_alloc(x);
}


int rejuv_node_alloc(int order)
{
    // assumption: bins[order]->next = NULL; bins[order] != NULL;
    //because current page is full occupied, so next of nalloc node will be in new page
    //new page will be allocated by balloc and init it with size captured by bins[order]
    struct node_alloc_node *next = buddy_alloc(4096);
    if (!next)
        return -1;
    node_alloc_init(next, 4096, bins[order]->size);
    bins[order]->next = next;
    uart_send_string("you are here for rejuv_nalloc\n");
    return 0;
}

void buddy_malloc_init()
{
    // merge balloc_nallocator together with bins[0]
    bins[0] = _global_allocator->nhead;
    for (int i = 1; i < sizeof(binsize)/sizeof(binsize[0]); i++) {
        void *m = buddy_alloc(4096);
        // m is location, 4096 is page_sz and binsize[i] is size for that allocator
        node_alloc_init(m, 4096, binsize[i]);
        bins[i] = m;
    }
    //after finish malloc_init balloc_nalloc become y_malloc
    balloc_nalloc = buddy_malloc;
}

void buddy_free(void *ptr) {

	uint32_t idx =((uint64_t)ptr) / _global_allocator->page_size;
	int32_t order = ((int32_t) _global_allocator->entries[idx] + 1) * -1;

	struct buddy_alloc_node *n = balloc_nalloc(sizeof(struct buddy_alloc_node));
	n->idx = idx;
	uint32_t neighbor_idx = n->idx ^ (1<<order);
	// check whether neighborhood in entries
	while(_global_allocator->entries[neighbor_idx] == order) {
		// find it from freelist
		struct buddy_alloc_node *c = _global_allocator->freelist[order], **prev_slot = &_global_allocator->freelist[order];
		while(c){
			if(c->idx == neighbor_idx) {
				struct buddy_alloc_node *merged = balloc_nalloc(sizeof(struct buddy_alloc_node));
				_global_allocator->entries[neighbor_idx] = IS_CHILD;
				merged->idx = MIN(n->idx,neighbor_idx);
				*prev_slot = c->next;
				nfree(_global_allocator->nhead,n);
				nfree(_global_allocator->nhead,c);
				n = merged;
				goto next_order;
			}
			prev_slot = &c->next;
			c = c->next;
		}
		while(1);
	next_order:
		order++;
		neighbor_idx = n->idx ^ (1<<order);
	}
	n->next = _global_allocator->freelist[order];
	_global_allocator->entries[n->idx] = (buddy_alloc_entry) order;
	_global_allocator->freelist[order] = n;
}



void node_free(void *ptr)
{
    if (__builtin_ctzll((uint64_t) ptr) >= 12) {
        uart_send_string("BFREE");ENDL;
        buddy_free(ptr);
        return;
    }
    struct node_alloc_node *header = ptr - sizeof(struct node_alloc_node);
    for (int i = 0; i < sizeof(binsize)/sizeof(binsize[0]); i++) {
        if (binsize[i] == header->size) {
            nfree(bins[i], ptr);
            return;
        }
    }
    uart_send_string("!!BUG!!\r\n");
    ENDL;

}


void buddy_system_init(void * _dtb) {
	
	char *dtb_addr = (char *)_dtb;
	
	//allocator location
    void *mem = (void *)0x10000000;
    //locate at 0x10000000
    struct buddy_alloc *bhead = mem;
    bhead->order = 11;
    bhead->page_size = 4096;
    bhead->size = 0x3C000000;
    buddy_alloc_preinit(bhead);
	
	uart_send_string("After buddy alloc preinit\n");

    
	buddy_alloc_reserve(0, (void *) 0x1000);
	buddy_alloc_reserve((void *) 0x1000, (void *) 0x80000);
    buddy_alloc_reserve((void *) 0x80000, (void *) 0x80000 + 30000);
    buddy_alloc_reserve(cpio_addr, cpio_addr + 30000);
    buddy_alloc_reserve(dtb_addr, dtb_addr + 60000);

	buddy_alloc_init();
	uart_send_string("\nAfter buddy alloc init\n");
	buddy_malloc_init();

	buddy_alloc_state(bhead);

	uart_send_string("\nallocate 5000bytes:\n");
	void* test1_malloc = buddy_malloc(5000);
	buddy_alloc_state(bhead);

	uart_send_string("\nfree 5000 bytes\n");
	node_free(test1_malloc);
	buddy_alloc_state(bhead);

	uart_send_string("\nallocate 10000bytes:\n");
	void* test2_malloc = buddy_malloc(10000);
	buddy_alloc_state(bhead);

	uart_send_string("\nfree 10000 bytes\n");
	node_free(test2_malloc);
	buddy_alloc_state(bhead);

}

