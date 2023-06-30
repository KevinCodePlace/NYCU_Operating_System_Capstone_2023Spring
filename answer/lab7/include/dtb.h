


typedef struct PROP{
    struct PROP* next;
    unsigned int len;
    struct {
        char prop_name[128];
        char prop_value[256];
    }prop_content;
}device_prop ;

typedef struct{
        char name[128];
        struct sub_device *next;
}sub_device;

typedef struct dtn{
    char device_name[128];
    device_prop* property;
    sub_device* sd;
}device_tree_node;

device_tree_node* initramfs_callback(char* path);
void *find_property_value(char* device_name, char* property_name);
