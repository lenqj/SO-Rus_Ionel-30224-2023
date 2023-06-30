#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define RESP_PIPE_NAME "RESP_PIPE_81807"
#define REQ_PIPE_NAME "REQ_PIPE_81807"
#define SH_MEM_NAME "/eev3QP"

typedef struct sectionStruct
{
    char *name;
    int type;
    int offset;
    int size;
} sectionStruct;
typedef struct sectionHeader
{
    char *magic;
    int header_size;
    int version;
    int no_of_sections;
    struct sectionStruct *sections;
} sectionHeader;

sectionHeader *parse(int fd, int flagFindAll)
{
    sectionHeader *header = (sectionHeader *)calloc(1, sizeof(sectionHeader));
    header->magic = (char *)calloc(3, sizeof(char));
    read(fd, header->magic, 2);
    header->magic[2] = '\0';
    if (strcmp(header->magic, "1c") != 0)
    {
        free(header);
        if (flagFindAll != 1)
        {
            printf("ERROR\nwrong magic\n");
        }
        return NULL;
    }
    read(fd, &header->header_size, 2);
    read(fd, &header->version, 2);
    if (header->version < 23 || header->version > 107)
    {
        if (flagFindAll != 1)
        {
            printf("ERROR\nwrong version\n");
        }
        free(header);
        return NULL;
    }
    read(fd, &header->no_of_sections, 1);
    if (header->no_of_sections < 8 || header->no_of_sections > 14)
    {
        if (flagFindAll != 1)
        {
            printf("ERROR\nwrong sect_nr\n");
        }
        free(header);
        return NULL;
    }
    int i = 0;
    header->sections = (sectionStruct *)calloc(header->no_of_sections, sizeof(sectionStruct));
    while (i < header->no_of_sections)
    {
        header->sections[i].name = (char *)calloc(20, sizeof(char));
        read(fd, header->sections[i].name, 19);
        header->sections[i].name[19] = '\0';
        read(fd, &header->sections[i].type, 4);
        if (header->sections[i].type != 10 && header->sections[i].type != 99 && header->sections[i].type != 46 && header->sections[i].type != 34)
        {
            free(header->sections);
            free(header);
            if (flagFindAll != 1)
            {
                printf("ERROR\nwrong sect_types\n");
            }
            return NULL;
        }
        read(fd, &header->sections[i].offset, 4);
        read(fd, &header->sections[i].size, 4);
        i++;
    }
    return header;
}

int main(void)
{
    int fdr, fdw, fdm, fd = -1;
    void *SH_MEM = NULL;
    void *SH_MEM_FILE = NULL;
    off_t size = -1;
    if (mkfifo(RESP_PIPE_NAME, 0600) != 0)
    {
        puts("ERROR\ncannot create the response pipe");
        return -1;
    }

    fdr = open(REQ_PIPE_NAME, O_RDONLY);
    if (fdr == -1)
    {
        puts("ERROR\ncannot open the request pipe");
        return -1;
    }

    fdw = open(RESP_PIPE_NAME, O_WRONLY);
    if (fdw == -1)
    {
        puts("ERROR\ncannot open the response pipe");
        return -1;
    }

    write(fdw, "CONNECT$", 8);
    puts("SUCCESS");
    while (1)
    {
        char request[100] = {'0'};
        int i = 0;
        while (i < 100)
        {
            read(fdr, &request[i], sizeof(request[i]));
            if (request[i] == '$')
            {
                break;
            }
            i++;
        }
        request[i] = '\0';

        if (strcmp(request, "ECHO") == 0)
        {
            unsigned int variant = 81807;
            write(fdw, "ECHO$", 5);
            write(fdw, "VARIANT$", 8);
            write(fdw, &variant, sizeof(variant));
        }
        else if (strcmp(request, "CREATE_SHM") == 0)
        {
            fdm = shm_open(SH_MEM_NAME, O_CREAT | O_RDWR, 0664);
            if (fdm < 0)
            {
                write(fdw, "CREATE_SHM$", 11);
                write(fdw, "ERROR$", 6);
            }
            else
            {
                unsigned int SH_MEM_SIZE;
                read(fdr, &SH_MEM_SIZE, sizeof(SH_MEM_SIZE));
                ftruncate(fdm, SH_MEM_SIZE);
                if (SH_MEM_SIZE == 2287168)
                {
                    SH_MEM = (void *)mmap(NULL, SH_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdm, 0);
                    if (SH_MEM == (void *)-1)
                    {
                        write(fdw, "CREATE_SHM$", 11);
                        write(fdw, "ERROR$", 6);
                    }
                    else
                    {
                        write(fdw, "CREATE_SHM$", 11);
                        write(fdw, "SUCCESS$", 8);
                    }
                }
                else
                {
                    write(fdw, "CREATE_SHM$", 11);
                    write(fdw, "ERROR$", 6);
                }
            }
        }
        else if (strcmp(request, "WRITE_TO_SHM") == 0)
        {
            unsigned int offset, value;
            read(fdr, &offset, sizeof(offset));
            read(fdr, &value, sizeof(value));
            if (offset < 0 || offset > 2287168 || offset + sizeof(value) < 0 || offset + sizeof(value) > 2287168)
            {
                write(fdw, "WRITE_TO_SHM$", 13);
                write(fdw, "ERROR$", 6);
            }
            else
            {
                memcpy(SH_MEM + offset, &value, sizeof(value));
                write(fdw, "WRITE_TO_SHM$", 13);
                write(fdw, "SUCCESS$", 8);
            }
        }
        else if (strcmp(request, "MAP_FILE") == 0)
        {
            char FILE_NAME[100] = {0};
            int i = 0;
            while (i < 100)
            {
                read(fdr, &FILE_NAME[i], sizeof(FILE_NAME[i]));
                if (FILE_NAME[i] == '$')
                {
                    break;
                }
                i++;
            }
            FILE_NAME[i] = '\0';
            fd = open(FILE_NAME, O_RDONLY);
            if (fd < 0)
            {
                write(fdw, "MAP_FILE$", 9);
                write(fdw, "ERROR$", 6);
            }
            else
            {
                size = lseek(fd, 0, SEEK_END);
                lseek(fd, 0, SEEK_SET);
                SH_MEM_FILE = (void *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
                if (SH_MEM_FILE == (void *)-1)
                {
                    write(fdw, "MAP_FILE$", 9);
                    write(fdw, "ERROR$", 6);
                }
                else
                {
                    write(fdw, "MAP_FILE$", 9);
                    write(fdw, "SUCCESS$", 8);
                }
            }
        }
        else if (strcmp(request, "READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned int offset, no_of_bytes;
            read(fdr, &offset, sizeof(offset));
            read(fdr, &no_of_bytes, sizeof(no_of_bytes));
            if (no_of_bytes < 0 || no_of_bytes > size || offset < 0 || offset > size || offset + no_of_bytes > size || SH_MEM_FILE == (void *)-1)
            {
                write(fdw, "READ_FROM_FILE_OFFSET$", 22);
                write(fdw, "ERROR$", 6);
            }
            else
            {
                memcpy(SH_MEM, SH_MEM_FILE + offset, no_of_bytes);
                write(fdw, "READ_FROM_FILE_OFFSET$", 22);
                write(fdw, "SUCCESS$", 8);
            }
        }
        else if (strcmp(request, "READ_FROM_FILE_SECTION") == 0)
        {
            unsigned int section_no, offset, no_of_bytes;
            read(fdr, &section_no, sizeof(offset));
            read(fdr, &offset, sizeof(offset));
            read(fdr, &no_of_bytes, sizeof(no_of_bytes));
            sectionHeader *header = (sectionHeader *)calloc(1, sizeof(sectionHeader));
            if (fd < 0)
            {
                header = NULL;
            }
            else
            {
                header = parse(fd, 1);
            }

            if (header == NULL || section_no > header->no_of_sections || section_no <= 0)
            {
                write(fdw, "READ_FROM_FILE_SECTION$", 23);
                write(fdw, "ERROR$", 6);
            }
            else
            {
                memcpy(SH_MEM, SH_MEM_FILE + header->sections[section_no - 1].offset + offset, no_of_bytes);
                write(fdw, "READ_FROM_FILE_SECTION$", 23);
                write(fdw, "SUCCESS$", 8);
            }
        }
        else if (strcmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            unsigned int logical_offset, no_of_bytes;
            read(fdr, &logical_offset, sizeof(logical_offset));
            read(fdr, &no_of_bytes, sizeof(no_of_bytes));
            sectionHeader *header = (sectionHeader *)calloc(1, sizeof(sectionHeader));
            if (fd < 0)
            {
                header = NULL;
            }
            else
            {
                header = parse(fd, 1);
            }

            if (header == NULL)
            {
                write(fdw, "READ_FROM_LOGICAL_SPACE_OFFSET$", 31);
                write(fdw, "ERROR$", 6);
            }
            else
            {
                int sizes[] = {0};
                for (int i = header->no_of_sections - 1; i > 0; i--)
                {
                    sizes[i] = header->sections[i - 1].size / 3072 + i;
                }
                for (int i = header->no_of_sections - 1; i >= 0; i--)
                {
                    memcpy(SH_MEM + sizes[i] * 3072, SH_MEM_FILE + header->sections[i].offset, no_of_bytes);
                }
                write(fdw, "READ_FROM_LOGICAL_SPACE_OFFSET$", 31);
                write(fdw, "SUCCESS$", 8);
            }
        }
        else if (strcmp(request, "EXIT") == 0)
        {
            close(fdr);
            close(fdw);
            unlink(REQ_PIPE_NAME);
            break;
        }
    }
    return 0;
}