#include "header/allocator.h"
#include "header/utils.h"

#define SIMPLE_MALLOC_BUFFER_SIZE 8192
static unsigned char simple_malloc_buffer[SIMPLE_MALLOC_BUFFER_SIZE];
static unsigned long simple_malloc_offset = 0;

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
