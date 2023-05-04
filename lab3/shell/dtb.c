#include "header/dtb.h"
#include "header/uart.h"
#include "header/utils.h"
#define UNUSED(x) (void)(x)


char* cpio_addr;
int space = 0;
//1. Define the callback function type(fdt_callback)
//2. Create a structure for holding the FDT header information(fdt_header)
//3. Implement helper functions to extract the FDT header information

uint32_t fdt_u32_le2be (const void *addr) {
	const uint8_t *bytes = (const uint8_t *) addr;
	uint32_t ret = (uint32_t)bytes[0] << 24 |(uint32_t)bytes[1] << 16 |(uint32_t)bytes[2] << 8 |(uint32_t)bytes[3];
	return ret;
}

void send_space(int n) {
	while(n--) uart_send_string(" ");
}

int parse_struct (fdt_callback cb, uintptr_t cur_ptr, uintptr_t strings_ptr,uint32_t totalsize) {
	uintptr_t end_ptr = cur_ptr + totalsize;
	
	while(cur_ptr < end_ptr) {
		
		uint32_t token = fdt_u32_le2be((char *)cur_ptr);
		cur_ptr += 4;
		
		switch(token){
			case FDT_BEGIN_NODE:
			/*
			Token type (4 bytes): Indicates that it's an FDT_BEGIN_NODE token.
			Node name (variable length, NULL-terminated): Specifies the name of the node being opened.
			*/
				//uart_send_string("In FDT_BEGIN_NODE\n");
				cb(token, (char*)cur_ptr,NULL,0);	
				cur_ptr += utils_align_up(utils_strlen((char*)cur_ptr),4);
				break;
			case FDT_END_NODE:
			/*
			Token type (4 bytes): Indicates that it's an FDT_END_NODE token.
			*/
				//uart_send_string("In FDT_END_NODE;\n");
				cb(token,NULL,NULL,0);
				break;

			case FDT_PROP: {

			/*
			Token type (4 bytes): Indicates that it's an FDT_PROP token.
			Data length (4 bytes): Specifies the length of the property data (len).
			Name offset (4 bytes): Provides the offset of the property name within the strings block (nameoff).
			Property data (variable length): Contains the property data itself, the size of which is determined by len.
			*/
				//uart_send_string("In FDT_PROP \n");	
				uint32_t len = fdt_u32_le2be((char*)cur_ptr);
				cur_ptr += 4;
				uint32_t nameoff = fdt_u32_le2be((char*)cur_ptr);
				cur_ptr += 4;
				//second parameter name here is property name not node name
				cb(token,(char*)(strings_ptr + nameoff),(void*)cur_ptr,len);
				cur_ptr += utils_align_up(len,4);
				break;

			}
			case FDT_NOP:
				//uart_send_string("In FDT_NOP\n");
				cb(token,NULL,NULL,0);
				break;

			case FDT_END:
				//uart_send_string("In FDT_END\n");
				cb(token,NULL,NULL,0);
				return 0;
			default:;
				return -1;
		}
	}
	return -1;

}
//4. Implement the fdt_traverse function:

int fdt_traverse(fdt_callback cb,void * _dtb){
	uintptr_t dtb_ptr = (uintptr_t) _dtb;
	uart_send_string("\ndtb loading at: ");
	uart_hex(dtb_ptr);
	uart_send_char('\n');

	struct fdt_header* header = (struct fdt_header*) dtb_ptr;
	
	uint32_t magic = fdt_u32_le2be(&(header->magic));	
	
	if (magic != 0xd00dfeed){
		
		uart_send_string("The header magic is wrong\n");
		return -1;
	}

	/*
	+-----------------+
	| fdt_header      | <- dtb_ptr
	+-----------------+
	| reserved memory |
	+-----------------+
	| structure block | <- dtb_ptr + header->off_dt_struct (struct_ptr)
	+-----------------+
	| strings block   | <- dtb_ptr + header->off_dt_strings (strings_ptr)
	+-----------------+
	 */

	uintptr_t struct_ptr = dtb_ptr + fdt_u32_le2be(&(header->off_dt_struct));
	uintptr_t strings_ptr = dtb_ptr + fdt_u32_le2be(&(header->off_dt_strings));
	uint32_t totalsize = fdt_u32_le2be(&header->totalsize);
	parse_struct(cb, struct_ptr,strings_ptr,totalsize);
	return 1;

}

//5. Implement the initramfs_callback function:
void get_cpio_addr(int token,const char* name,const void* data,uint32_t size){
	UNUSED(size);
	if(token==FDT_PROP && utils_string_compare((char *)name,"linux,initrd-start")){
		cpio_addr = (char*)(uintptr_t)fdt_u32_le2be(data);
		uart_send_string("cpio address is at: ");
		uart_hex((uintptr_t)fdt_u32_le2be(data));
		uart_send_char('\n');
	}
}

//6. Implement print_dtb callback function:

void print_dtb(int token, const char* name, const void* data, uint32_t size) {
	UNUSED(data);
	UNUSED(size);

	switch(token){
		case FDT_BEGIN_NODE:
			uart_send_string("\n");
			send_space(space);
			uart_send_string((char*)name);
			uart_send_string("{\n ");
			space++;
			break;
		case FDT_END_NODE:
			uart_send_string("\n");
			space--;
			if(space >0) send_space(space);
			uart_send_string("}\n");
			break;
		case FDT_NOP:
			break;
		case FDT_PROP:
			send_space(space);
			uart_send_string((char*)name);
			break;
		case FDT_END:
			break;
	}
}
