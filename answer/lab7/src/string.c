#include "uint.h"
void itoa(unsigned long long value,char *s) {
    int idx = 0;
    char tmp[24];
    int tidx = 0;
    do {
        tmp[tidx++] = '0' + value % 10;
        value /= 10;
    } while (value != 0 && tidx < 11);
    // reverse tmp
    int i;
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';
}

void ftoa(float *value, int precise, char *s) {
    int temp=1;
    for(int i=0;i<precise;i++) temp *= 10;
    long long in = (*value)*temp;
    char int_part[10],float_part[10];
    itoa(in/temp,int_part);
    itoa(in%temp,float_part); 
    strccat(int_part,".\0",s);
    strccat(s,float_part,s);
}

void i16toa(unsigned int value, char *s, int digits) {
    int idx = 0;
    char tmp[11];
    int tidx = 0;
    do {
        if((value % 16) < 10)
          tmp[tidx++] = '0' + value % 16;
        else
          tmp[tidx++] = 'A' + value % 16 - 10;
        value /= 16;
    } while (value != 0 && tidx < 11);
    // reverse tmp
    int i;
    for(int j=0;j<digits-tidx;j++) s[idx++]='0';
    for (i = tidx - 1; i >= 0; i--) {
        s[idx++] = tmp[i];
    }
    s[idx] = '\0';
}

int strcmp(char *string1, char *string2){
    while(*string1 && *string2){
        if(*string1!=*string2) return 0;
        else{
            string1++;
            string2++;
        }
    }
    if(*string1){
        if((*string1>='a' && *string1<='z') || (*string1>='A' && *string1<='Z') || (*string1>='0' && *string1<='9'))
            return 0;
        else
            return 1;
    }
    if(*string2){
        if((*string2>='a' && *string2<='z') || (*string2>='A' || *string2<='Z') || (*string2>=0 && *string2<=9))
            return 0;
        else
            return 1;
    }
    return 1;
}

int strncmp(char *string1, char *string2, int length){
    char temp1[128],temp2[128];
    for(int i=0;i<length;i++){
        temp1[i]=string1[i];
        temp2[i]=string2[i];
    }
    temp1[length]='\0';
    temp2[length]='\0';
    return strcmp(temp1,temp2);
}

void strcpy(const char *string1, char* string2, int length){
    int i;
    for(i=0;i<length && string1[i] != '\0';i++) string2[i]=string1[i];
}

uint64 letobe(uint64 o){
    byte *temp = (byte*) &o;
    uint64 result=0;
    for(int i=0;i<4;i++){
        result *= 256;
        result += temp[i];
    }
    
    return result;
}

void strccat(char* s1, char* s2, char* out){
    int i=0,j=0;
    for(;s1[i]>=32 && s1[i]<=127;i++){
        out[i]=s1[i];
    }
    for(;s2[j]>=32 && s2[j]<=127;j++){
        out[i+j]=s2[j];
    }
    out[i+j]=0;

}

int atoi(char *s){
    int n=0;
    for(int i=0;s[i]>=32 && s[i]<=127;i++){
        n*=10;
        n += s[i]-'0';
    }
    return n;
}

int a16toi(char *s){
    int n=0;
    for(int i=0;(s[i]>='0' && s[i]<='9') || (s[i]>='A' && s[i]<='F') || (s[i]>='a' && s[i]<='f');i++){
        n*=16;
        if(s[i]>='0' && s[i]<='9')
            n += s[i]-'0';
        else if(s[i]>='A' && s[i]<='F')
            n += s[i]-'A';
        else if(s[i]>='a' && s[i]<='f')
            n += s[i]-'a';
    }
    return n;
}

int a16ntoi(char *num, int length){  // transform hex string to int
    int namesize=0;
    for(int i=0;i<length;i++){
        namesize <<= 4;
        if(num[i]>='0' && num[i]<='9'){
            namesize += (num[i]-'0');
        }else if(num[i]>='A' && num[i]<='F'){
            namesize += (num[i]-'A'+10);
        }
    }
    return namesize;
}

void *memset(void *str, int c, size_t n){
    unsigned char* string = (unsigned long)str + c;
    for(int i=0;i<n;i++) *string++ = 0;
    return str;
}

char * strtok(char * str, const char * delimiters){
    static char* record_str = 0;
    static char return_word[128];
    static int i = 0;
    if(str != NULL) record_str = str;
    else if(str == NULL && record_str == 0) return NULL;
    int idx = 0;
    memset(return_word,0,128);
    for(;i<256 && str[i]!=0;i++){
        int n=0;
        for(int j=0;j<256 && delimiters[j]!=0;j++){
            if(str[i] == delimiters[j]){
                if(idx!=0) return return_word;
                else{
                    n=1;
                    break;
                }
            }
        }
        if(n==0){
            return_word[idx++] = str[i];
        }
    }
    if(idx!=0) return return_word;
    else return NULL;
}
