#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include "a2_helper.h"

void *thf(void *arg)
{
    int s = *(int *)arg;
    info(BEGIN, 7, s);
    info(END, 7, s);
    //exit(0);
    return NULL;
}
void *thf2(void *arg)
{
    int s = *(int *)arg;
    info(BEGIN, 6, s);
    info(END, 6, s);
    //exit(0);
    return NULL;
}
void *thf3(void *arg)
{
    int s = *(int *)arg;
    info(BEGIN, 3, s);
    info(END, 3, s);
    //exit(0);
    return NULL;
}

int main()
{
    init();

    info(BEGIN, 1, 0);
    if (fork() == 0)
    {
        info(BEGIN, 2, 0);
        if (fork() == 0)
        {
            info(BEGIN, 3, 0);

            pthread_t threads[6];
                int v[6];
                for (int i = 0; i < 6; i++)
                {
                    v[i] = i+1;
                    pthread_create(&threads[i], NULL, thf3, &v[i]);
                }   
                for (int i = 0; i < 6; i++)
                {
                    pthread_join(threads[i], NULL);
                }
            if (fork() == 0)
            {
                info(BEGIN, 4, 0);
                info(END, 4, 0);
                exit(0);
            }
            else
            {
                wait(NULL);
                // wait(NULL);
            }
            if (fork() == 0)
            {
                info(BEGIN, 6, 0);
                pthread_t threads[37];
                int v[37];
                for (int i = 0; i < 37; i++)
                {
                    v[i] = i+1;
                    pthread_create(&threads[i], NULL, thf2, &v[i]);
                }   
                for (int i = 0; i < 37; i++)
                {
                    pthread_join(threads[i], NULL);
                }
                info(END, 6, 0);
                exit(0);
            }
            else
            {
                wait(NULL);
                // wait(NULL);
            }
            info(END, 3, 0);
            exit(0);
        }
        else
        {
            wait(NULL);
            // wait(NULL);
        }
        if (fork() == 0)
        {
            pthread_t threads[4];
            info(BEGIN, 7, 0);
            int v[] = {1, 2, 3, 4};
            for (int i = 0; i < 4; i++)
            {
                pthread_create(&threads[i], NULL, thf, &v[i]);
            }
            for (int i = 0; i < 4; i++)
            {
                pthread_join(threads[i], NULL);
            }
            info(END, 7, 0);
            exit(0);
        }
        else
        {
            wait(NULL);
            // wait(NULL);
        }
        info(END, 2, 0);
        exit(0);
    }
    else
    {
        wait(NULL);
        // wait(NULL);
    }
    if (fork() == 0)
    {
        info(BEGIN, 5, 0);
        info(END, 5, 0);
        exit(0);
    }
    else
    {
        wait(NULL);
        // wait(NULL);
    }
    info(END, 1, 0);
    exit(0);

    return 0;
}