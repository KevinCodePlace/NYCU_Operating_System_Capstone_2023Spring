#include "uint.h"
#include "dtb.h"
#include "fdt.h"
#include "mini_uart.h"
#include "string.h"
#include "allocator.h"

extern byte __dtb_addr;

int acceptable_word(char con){
    if(((con >= 'A' && con<='Z') || (con >= 'a' && con <= 'z') || (con >= '0' || con <= '9') || con == '.' || con == '/')) return 1;
    else return 0;
}

void *find_property_value(char* device_name, char* property_name){
    device_tree_node *node;
    node = initramfs_callback(device_name);
    device_prop *prop = node->property;
    uint32 *p = prop->prop_content.prop_name;
    while( *p != 0){
        if(strcmp(property_name, prop->prop_content.prop_name)==1){
            return prop->prop_content.prop_value;
        }
        prop= prop->next;
        p=prop->prop_content.prop_name;
    }
    uart_printf("Error! Device or Property not found.\n");
    return (void*)0;
}

void print_properties(device_prop* node, int level){ 
    uint32 *p = (uint32*) (node->prop_content).prop_name;
    for(int i=0;i<level;i++) uart_printf(" ");
    uart_printf(" property:\n");
    int address_cells=2,size_cells=2,interrupt_cells=1;
    while(*p!=0){
        for(int i=0;i<level;i++) uart_printf(" ");
        uart_printf("  %s : ",(node->prop_content).prop_name);
        if(strcmp(node->prop_content.prop_name,"compatible\0")){
            uart_write(34);
            int is_bottom=0;
            for(int i=0;i<node->len;i++){
                if(node->prop_content.prop_value[i]!='\0'){
                    if(is_bottom==1){
                        uart_printf(",\"");
                        is_bottom=0;
                    }
                    uart_write(node->prop_content.prop_value[i]);
                }else{
                    uart_printf("\"");
                    is_bottom=1;
                }
            }
            uart_printf("\n");
        }else if(strcmp(node->prop_content.prop_name,"model\0") ||
         strcmp(node->prop_content.prop_name,"status\0") ||
         strcmp(node->prop_content.prop_name,"name\0") ||
         strcmp(node->prop_content.prop_name,"device_type\0")){
            uart_printf("\"%s\"\n",node->prop_content.prop_value);
        }else if(strcmp(node->prop_content.prop_name,"phandle\0") ||
         strcmp(node->prop_content.prop_name,"#address-cells\0") ||
         strcmp(node->prop_content.prop_name,"#size-cells\0") ||
         strcmp(node->prop_content.prop_name,"virtual-reg\0")){
            uint32 *temp = node->prop_content.prop_value;
            int value=letobe(*temp);
            if(strcmp(node->prop_content.prop_name,"#address-cells\0")) address_cells=value;
            else if(strcmp(node->prop_content.prop_name,"#size-cells\0")) size_cells=value;
            uart_printf("<%d>\n",value);
        }else if(strcmp(node->prop_content.prop_name,"reg\0")){
            uart_printf("<");
            uint32 *temp = node->prop_content.prop_value;
            for(int i=0;i<node->len;){
                if(address_cells == 1){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x", letobe(*temp2));
                    temp++;
                    i+=4;
                }else if(address_cells == 2){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x %x", letobe(temp2[0]),letobe(temp2[1]));
                    temp+=2;
                    i+=8;
                }
                if(i>=node->len) break;
                if(size_cells == 1){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x", letobe(*temp2));
                    temp++;
                    i+=4;
                }else if(size_cells == 2){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x %x", letobe(temp2[0]),letobe(temp2[1]));
                    temp+=2;
                    i+=8;
                }
            }
            uart_printf(" >\n");
        }else if(strcmp(node->prop_content.prop_name,"ranges\0") ||
         strcmp(node->prop_content.prop_name,"dma-ranges\0")){
            if(node->len != 0){
                uart_printf("<");
                uint32 *temp = node->prop_content.prop_value;
                if(address_cells == 1){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x 0x%x", letobe(*temp2),letobe(*(temp2+1)));
                    temp+=2;
                }else if(address_cells == 2){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x %x 0x%x %x", letobe(temp2[0]),letobe(temp2[1]),letobe(temp2[3]),letobe(temp2[4]));
                    temp+=4;
                }
            
                if(size_cells == 1){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x", letobe(*temp2));
                    temp++;
                }else if(size_cells == 2){
                    uint32 *temp2 = temp;
                    uart_printf(" 0x%x%x", letobe(temp2[0]),letobe(temp2[1]));
                    temp+=2;
                }
                uart_printf(" >\n");
            }else uart_printf("\n");
        }else if(strcmp(node->prop_content.prop_name,"interrupts\0")){
            uint32 *temp=node->prop_content.prop_value;
            uart_printf("<0x%x %d>\n",letobe(temp[0]),letobe(temp[1]));
        }else if(strcmp(node->prop_content.prop_name,"interrupt-parent\0")){
            uint32 *temp=node->prop_content.prop_value;
            uart_printf("<%d>\n",letobe(temp[0]));
        }else if(strcmp(node->prop_content.prop_name,"interrupts-extended\0")){
            uint32 *temp=node->prop_content.prop_value;
            //uart_printf("<%d>\n",letobe(temp[0]));
        }else if(strcmp(node->prop_content.prop_name,"interrupt-cells\0")){
            uint32 *temp=node->prop_content.prop_value;
            uart_printf("<%d>\n",letobe(temp[0]));
        }else if(strcmp(node->prop_content.prop_name,"linux,initrd-end\0") ||
         strcmp(node->prop_content.prop_name,"linux,serial\0") ||
         strcmp(node->prop_content.prop_name,"linux,initrd-start\0") ||
         strcmp(node->prop_content.prop_name,"linux,revision\0")){
            uint32 *temp=node->prop_content.prop_value;
            uart_printf("0x%x\n",letobe(temp[0]));
        }else if(strcmp(node->prop_content.prop_name,"size\0") ||
         strcmp(node->prop_content.prop_name,"clock-frequency\0") ||
         strcmp(node->prop_content.prop_name,"timebase-frequency\0")){
            uint32 *temp=node->prop_content.prop_value;
            if(size_cells == 1)
                uart_printf("<0x%x>\n",letobe(temp[0]));
            else if(size_cells == 2)
                uart_printf("<0x%x %x>\n",letobe(temp[0]),letobe(temp[1]));
        }else{
            if(node->len == 0 ){
                char temp[4]={8,8,' ','\0'};
                uart_printf("%s\n",temp);
            }
            else{
                char *temp2=node->prop_content.prop_value;
                if(acceptable_word(temp2[0]) && acceptable_word(temp2[1])){
                    uart_printf("\"%s\"\n",temp2);
                }else{
                    uart_printf("0x");
                    uint32 *temp = node->prop_content.prop_value;
                    int t=node->len%4==0? node->len/4 : node->len/4+1;
                    for(int j=0;j<t;j++){
                        uart_printf("%x ",letobe(temp[j]));
                    }
                    uart_printf("\n");
                }
                
            }
        }
        
        node=node->next;
        p=(uint32*) (node->prop_content).prop_name;
    }
}


void traverse_tree(char* start,int level){ //print start's properties and its sub_device's properties
    device_tree_node *node;
    node = initramfs_callback(start);
    sub_device *now = node->sd;
    
    for(int i=0;i<level;i++) uart_printf(" ");
    uart_printf("%s\n",node->device_name);
    print_properties(node->property,level+1);
    uint32 *n = (uint32*) now->name;
    while(*n!=0){
        char next[128];
        int j;
        for(j=0;j<128;j++){
            next[j] = start[j];
            if(start[j] == '\0') break;
        }
        if(start[j-1] != '/'){
            next[j++]='/';
            next[j]='\0';
        }
        for(int k=0;k<128;k++){
            next[k+j] = now->name[k];
            if(now->name[k] == '\0') break;
        }
        traverse_tree(next, level+1);
        now=now->next;
        n=(uint32*) now->name;
        
    }
    return;
}

device_tree_node* initramfs_callback(char* path){ //looking for device
    uint32* addr = (uint32*) &__dtb_addr;
    device_tree_node *node = simple_malloc(sizeof(device_tree_node));
    sub_device *temp_node=simple_malloc(sizeof(sub_device));
    device_prop* temp_prop=simple_malloc(sizeof(device_prop));
    node->property = temp_prop;
    node->sd=temp_node;
    for(int i=0;i<4;i++){
        (temp_node->name)[i]=0;
        ((temp_prop->prop_content).prop_name)[i]=0;
    }
    if(path[0] != '/'){
        uart_printf("Device not fount: %s\n", path);
        return node;
    }else{
        fdt_header *fh=(fdt_header*)(*addr);
        uint32* struct_addr = (uint32*) (*addr + letobe(fh->off_dt_struct));
        uint32* string_addr = (uint32*) (*addr + letobe(fh->off_dt_strings));
        int now=0,level=0;
        char name[128];
        while(path[now++] != '\0' && path[now] != '\0'){
            int i=0;
            for(i=0;path[now+i]!='\0' && path[now+i]!='/';i++){
                name[i]=path[now+i];
            }
            now+=i;
            name[i]='\0';
            while(1){
                int struct_block=letobe(*struct_addr++);
                if(struct_addr >= string_addr){
                    uart_printf("Device not found: %s\n",path);
                    return node; 
                } 
                if(struct_block == 1){
                    if(level == 1){
                        char device_name[128];
                        char *temp = (char*) (struct_addr);
                        int j;
                        for(j=0;temp[j]!=0;j++){
                            device_name[j]=temp[j];
                        }
                        device_name[j]='\0';
                        
                        if(strcmp(device_name, name)){
                            break;
                        }else{
                            struct_addr += ( j%4 ==0 ? j/4:j/4+1);
                        }
                    }else{
                        char *temp = (char*) (struct_addr);
                        int j;
                        for(j=0;temp[j]!=0;j++);
                        struct_addr += ( j%4 ==0 ? j/4:j/4+1);
                    }
                    level++;
                }else if(struct_block == 2){
                    level--;
                    if(level == 0){
                        uart_printf("Device not found: %s\n",path);
                        return node;
                    }
                }else if(struct_block == 3){
                    FDT_PROP *pro = (FDT_PROP*) struct_addr;
                    uint32 len = letobe(pro->len)%4==0? letobe(pro->len)/4 : letobe(pro->len)/4+1;
                    struct_addr += ( 2 + len);
                }else if(struct_block == 9){
                    uart_printf("Device not found: %s\n",path);
                    return node;
                }
            }
        }
        if(now-1<=1) strcpy("/\0", node->device_name, 1);
        else strcpy(path,node->device_name,now-1);
        level=0;
        struct_addr--;
        while(1){
            if(struct_addr >= string_addr) return node;
            int struct_block=letobe(*struct_addr++);
            if(struct_block == 1){
                if(level == 1){
                    char device_name[128];
                    char *temp = (char*) (struct_addr);
                    int j=0;
                    for(j=0;temp[j]!=0;j++){
                        device_name[j]=temp[j];
                    }
                    device_name[j]='\0';
                    struct_addr += ( j%4 ==0 ? j/4:j/4+1);
                    strcpy(device_name,temp_node->name,j);
                    sub_device *node_t=simple_malloc(sizeof(sub_device));
                    for(int l=0;l<4;l++){
                        node_t->name[l]=0;
                    }
                    temp_node->next=node_t;
                    temp_node=node_t;
                    
                }else{
                    char *temp = (char*) (struct_addr);
                    int j=0;
                    for(;temp[j]!=0;j++);
                    struct_addr += ( j%4 ==0 ? j/4:j/4+1);
                }
                level++;
            }else if(struct_block == 2){
                level--;
                if(level == 0){
                    return node;
                }
            }else if(struct_block == 3){
                if(level == 1){
                    char* name = (char*)string_addr + letobe(struct_addr[1]);
                    device_prop *prop = simple_malloc(sizeof(device_prop));
                    int l=0;
                    for(l=0;name[l] != 0; l++){
                        (temp_prop->prop_content).prop_name[l]=name[l];
                    }
                    (temp_prop->prop_content).prop_name[l]='\0';
                    char *t=&(struct_addr[2]);
                    for(l=0;l<letobe(struct_addr[0]);l++){
                        (temp_prop->prop_content).prop_value[l]=t[l];
                    }
                    (temp_prop->prop_content).prop_value[l]='\0';
                    temp_prop->len=letobe(struct_addr[0]);
                    for(l=0;l<4;l++) (prop->prop_content).prop_name[l]='\0';
                    temp_prop->next=prop;
                    temp_prop=prop;
                }
                uint32 len = letobe(struct_addr[0])%4==0? letobe(struct_addr[0])/4 : letobe(struct_addr[0])/4+1;
                struct_addr += ( 2 + len);
            }else if(struct_block == 9){
                return node;
            }
        }
        
    }

}

