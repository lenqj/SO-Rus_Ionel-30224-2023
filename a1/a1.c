#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("81807\n");
        }
    }
    return 0;
}