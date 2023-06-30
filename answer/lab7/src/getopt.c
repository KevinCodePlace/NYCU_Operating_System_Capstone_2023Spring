#include "uint.h"
#include "mini_uart.h"

int optind=1;
char* optarg;

char *getopt(int argc, char* argv[],char* opt){
    char param[16];
    int ind = 0;
    for(int i=0;opt[i] != 0;i++){
        if(opt[i] == ':') param[ind++] = opt[i+1];
    }
    for(;optind<argc;optind++){
        if(argv[optind][0] == '-'){
            for(int i=0;i<ind;i++){
                if(argv[optind][1] == param[i]){
                    if( optind+1 < argc && argv[optind+1][0] != '-') optarg = argv[optind+1];
                    else optarg = NULL;
                    return argv[optind++][1];
                }
            }
        }
    }
    return 0;
}