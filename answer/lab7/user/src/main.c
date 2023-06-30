#include "system.h"

int main(){
    char a[] = "How are you\n";
    uart_write(a,sizeof(a));
    exit();
}