extern unsigned char _begin_, _end_, __relocation;
__attribute__((section(".text.relocator")))void relocate(void){
    unsigned long long  length = ( &_end_ - &_begin_);
    unsigned char* addr_after = (unsigned char*) &__relocation;
    unsigned char* addr_before = (unsigned char*) &_begin_;
    for(unsigned int i=0; i<length ; i++){
        *(addr_after+i) = *(addr_before+i);
        *(addr_before+i) = 0;
    }
    void (*startos)(void)=(void*)addr_after;
    startos();
}