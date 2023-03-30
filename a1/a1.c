#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
int listCurrent(const char *path, char* name_starts_with, int has_perm_write, int recursive)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    dir = opendir(path);
    if(dir == NULL) {
        puts("ERROR");
        puts("invalid directory path");
        return 0;
    }
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, ".." ) != 0 ) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(name_starts_with != NULL){
                    if(strncmp(entry->d_name, name_starts_with, strlen(name_starts_with)) == 0){
                        if(has_perm_write == 0){
                            printf("%s\n", fullPath);
                        }else{
                            if((statbuf.st_mode & S_IWUSR) == S_IWUSR){
                                printf("%s\n", fullPath);
                            }
                        }
                    }
                }else{
                    if(has_perm_write == 1){
                        if((statbuf.st_mode & S_IWUSR) == S_IWUSR){
                            printf("%s\n", fullPath);
                        }
                    }else{
                        printf("%s\n", fullPath);
                    }
                }
                if(recursive == 1){
                    if(S_ISDIR(statbuf.st_mode)){
                        listCurrent(fullPath, name_starts_with, has_perm_write, recursive);
                    }
                }
            }
        }
    }
    closedir(dir);
    return 1;
}
int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("81807\n");
        }
        char *auxPath = NULL;
        char *auxName = NULL;
        int has_perm_write = 0;
        int recursive = 0;
        if (strcmp(argv[1], "list") == 0)
        {
            if(strcmp(argv[2], "recursive")  == 0){
                recursive = 1;
                auxPath = argv[4];
                if(strncmp(argv[3], "name_starts_with=", 17)  == 0){
                    auxName = argv[3];
                    has_perm_write = 0;
                }
                if(strncmp(argv[3], "has_perm_write", 14)  == 0){
                    auxName = NULL;
                    has_perm_write = 1;
                }
                if(strncmp(argv[3], "path=", 5)  == 0){
                    auxPath = argv[3];
                }
            }else 
            if(strncmp(argv[2], "path=", 5)  == 0){
                    auxPath = argv[2];
            }else{
                recursive = 0;
                auxPath = argv[2];
                if(strncmp(argv[2], "name_starts_with=", 17)  == 0){
                    auxName = argv[2];
                    has_perm_write = 0;
                }
                if(strncmp(argv[2], "has_perm_write", 14)  == 0){
                    auxName = NULL;
                    has_perm_write = 1;
                }
                if(strncmp(argv[3], "path=", 5)  == 0){
                    auxPath = argv[3];
                }
            }
            char *path = (char*)calloc(strlen(auxPath), sizeof(char));
            char *name_starts_with = NULL;
            for(int i = 0; i < strlen(auxPath);i++){
                path[i] = auxPath[i+5];
            }
            if(auxName != NULL){
                name_starts_with = (char*)calloc(strlen(auxName), sizeof(char));
                for(int i = 0; i < strlen(auxName); i++){
                    name_starts_with[i] = auxName[i+17];
                }
            }
            puts("SUCCESS");
            listCurrent(path, name_starts_with, has_perm_write, recursive);
        }
        if (strcmp(argv[1], "parse") == 0 && strncmp(argv[2], "path=", 5)  == 0)
        {
           
            
        }
    }
    return 1;
}