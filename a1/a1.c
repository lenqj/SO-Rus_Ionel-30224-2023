#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct sectionStruct{
    char name[20];
    int type;
    int offset;
    int size;
}sectionStruct;
typedef struct sectionHeader{
    char magic[3];
    int header_size;
    int version;
    int no_of_sections;
    struct sectionStruct *sections;
}sectionHeader;

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

sectionHeader* parse(int fd, int flagParse){
    sectionHeader *header = (sectionHeader*)calloc(1, sizeof(sectionHeader));
    read(fd, &header->magic, 2);
    header->magic[2] = '\0';
    if(strcmp(header->magic, "1c") != 0){
        printf("ERROR\nwrong magic\n");
        return NULL;
    }
    read(fd, &header->header_size, 2);
    read(fd, &header->version, 2);
    if(header->version < 23 || header->version > 107){
        printf("ERROR\nwrong version\n");
        return NULL;
    }
    read(fd, &header->no_of_sections, 1);
    if(header->no_of_sections < 8 || header->no_of_sections > 14){
        printf("ERROR\nwrong sect_nr\n");
        return NULL;
    }
    int i = 0;
    header->sections = (sectionStruct*)calloc(header->no_of_sections, sizeof(sectionStruct));
    while(i < header->no_of_sections){
        read(fd, &header->sections[i].name, 19);
        header->sections[i].name[19] = '\0';
        read(fd, &header->sections[i].type, 4);
        if (header->sections[i].type != 10 && header->sections[i].type != 99 && header->sections[i].type != 46 && header->sections[i].type != 34){
			printf("ERROR\nwrong sect_types\n");
			return NULL;
		}
        read(fd, &header->sections[i].offset, 4);
        read(fd, &header->sections[i].size, 4);
        i++;
    }
    return header;
}
void printSection(sectionHeader* header){
        int i = 0;
        printf("SUCCESS\n");
        printf("version=%d\n", header->version);
        printf("nr_sections=%d\n", header->no_of_sections);
        while(i < header->no_of_sections){
            printf("section%d: %s %d %d\n", i+1, header->sections[i].name, header->sections[i].type, header->sections[i].size);                
            i++;
        }  
}

void extract(int fd, int section, int line){
    lseek(fd, 0, SEEK_SET);
    //sectionStruct *sections = parse(fd, 1);
    //lseek(fd, sections[section].offset, SEEK_SET);

    char c[2];
    int nr = 0;
    while(read(fd, &c, 1) && line > 0){
        if((strcmp(c, "\x0D\x0A")) == 0){
            nr++;
        }
        if(nr == (line))
            break;
    }
    char buff;
    while(read(fd, &buff, 1) > 0){
        printf("%c", buff);
    }
    printf("\n");
    //free(sections);
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
            free(path);
            free(name_starts_with);
        }
        if (strcmp(argv[1], "parse") == 0 && strncmp(argv[2], "path=", 5)  == 0)
        {
            char *auxPath = argv[2];
            char *path = (char*)calloc(strlen(auxPath), sizeof(char));
            for(int i = 0; i < strlen(auxPath);i++){
                path[i] = auxPath[i+5];
            }
            int fd = -1;
            if((fd = open(path, O_RDONLY)) <= 0)
            {
                printf("ERROR\ninvalid file");
            }else{
                sectionHeader *header = parse(fd, 1);
                if(header!=NULL){
                    printSection(header);
                }
            }
            free(path);
            close(fd);
        }
        if (strcmp(argv[1], "extract") == 0 && strncmp(argv[2], "path=", 5)  == 0 && strncmp(argv[3], "section=", 8)  == 0 && strncmp(argv[4], "line=", 5)  == 0)
        {
            char *auxPath = argv[2];
            char *auxSection = argv[3];
            char *auxLine = argv[4];
            char *path = (char*)calloc(strlen(auxPath), sizeof(char));
            char *section = (char*)calloc(strlen(auxSection), sizeof(char));
            char *line = (char*)calloc(strlen(auxLine), sizeof(char));

            for(int i = 0; i < strlen(auxPath);i++){
                path[i] = auxPath[i+5];
            }
            for(int i = 0; i < strlen(auxSection);i++){
                section[i] = auxSection[i+8];
            }
            for(int i = 0; i < strlen(auxLine);i++){
                line[i] = auxLine[i+5];
            }
            int fd;
            if((fd = open(path, O_RDONLY)) <= 0)
            {
                printf("ERROR\ninvalid file");
            }else{
                int sectionInt = atoi(section);
                int lineInt = atoi(line);
                extract(fd, sectionInt, lineInt);
            }
            free(path);
            free(section);
            free(line);
            close(fd);
        }

    }
    return 1;
}
