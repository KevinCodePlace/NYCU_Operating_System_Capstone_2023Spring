int log(int base, int logarithm){
    int tmp = 1;
    int exp = 0;
    while(tmp <= logarithm){
        tmp *= base;
        exp ++;
    }
    return exp-1;
}

int exp(int base, int exponent){
    int tmp = 1;
    for(int i=0;i<exponent;i++){
        tmp *= base;
    }
    return tmp;
}

int log2(int logarithm){
    return log(2,logarithm);
}
int exp2(int exponent){
    return exp(2,exponent);
}

long long upper_bound(long long dividend, long long divisor){
    return dividend%divisor == 0 ? dividend/divisor : dividend/divisor+1;
}

unsigned long long min(unsigned long long a,unsigned long long b){
    return a>=b ? b:a;
}