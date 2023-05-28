#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define RESP_PIPE_NAME "RESP_PIPE_81807"
#define REQ_PIPE_NAME "REQ_PIPE_81807"

int main(void)
{

    int fdr, fdw = -1;
    char connect_message[] = "CONNECT$";
    int l = strlen(connect_message) + 1;
    if (mkfifo(RESP_PIPE_NAME, 0600) != 0)
    {
        perror("ERROR\ncannot create the response pipe");
        return 1;
    }

    fdr = open(REQ_PIPE_NAME, O_RDONLY);
    if (fdr == -1)
    {
        perror("ERROR\ncannot open the request pipe");
        return 1;
    }

    fdw = open(RESP_PIPE_NAME, O_WRONLY);

    write(fdw, connect_message, l);

    sleep(1);

    char response[100];
    read(fdr, response, sizeof(response));

    if (strcmp(response, "SUCCESS$") == 0)
    {
        printf("SUCCESS\n");
    }
    else
    {
        exit(1);
    }

    // while (1)
    // {
    //     char request[1024];
    //     read(fdr, request, sizeof(request));

    //     if (strcmp(request, "ECHO$") == 0)
    //     {
    //         char response_message[] = "ECHO$VARIANT$81807";
    //         write(fdw, response_message, strlen(response_message) + 1);
    //     }
    //     if (strcmp(request, "EXIT$") == 0)
    //     {
    //         close(fdr);
    //         close(fdw);
    //         exit(1);
    //     }
    // }

    return 0;
}