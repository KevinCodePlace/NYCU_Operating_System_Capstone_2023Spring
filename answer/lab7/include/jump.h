struct JumpBuf{
    long long int reg[32];
};

int setjump(struct JumpBuf* jb);
int longjump(struct JumpBuf* jb, int return_value);