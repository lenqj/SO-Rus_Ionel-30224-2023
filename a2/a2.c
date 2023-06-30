#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "a2_helper.h"
typedef struct THS
{
    int value;
    sem_t *logSem[2];
} THS;

int running_threads = 0;
#define MAX_THREADS 6

void *thf(void *arg)
{
    THS *s = (THS*)arg;
    if (s->value == 2)
    {
        info(BEGIN, 7, s->value);
        sem_post(s->logSem[0]);
        sem_wait(s->logSem[1]);
        info(END, 7, s->value);
    }
    else if (s->value == 4)
    {
        sem_wait(s->logSem[0]);
        info(BEGIN, 7, s->value);
        info(END, 7, s->value);
        sem_post(s->logSem[1]);
    }
    else
    {
        info(BEGIN, 7, s->value);
        info(END, 7, s->value);
    }
    // exit(0);
    return NULL;
}
void *thf2(void *arg)
{
    THS *s = (THS*)arg;
    
    sem_wait(s->logSem[0]);

    info(BEGIN, 6, s->value);
    info(END, 6, s->value);

    sem_post(s->logSem[0]);
    // exit(0);
    return NULL;
}
void *thf3(void *arg)
{
    int s = *(int *)arg;
    info(BEGIN, 3, s);
    info(END, 3, s);
    // exit(0);
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
                v[i] = i + 1;
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
                THS params[37];
                sem_t logSem;
                sem_init(&logSem, 0, 6);
                for (int i = 0; i < 37; i++)
                {
                    params[i].value = i + 1;
                    params[i].logSem[0] = &logSem;
                    pthread_create(&threads[i], NULL, thf2, &params[i]);
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
            info(BEGIN, 7, 0);
            pthread_t threads[4];
            THS params[4];
            sem_t logSem[2];
            sem_init(&logSem[0], 0, 0);
            sem_init(&logSem[1], 0, 0);
            for (int i = 0; i < 4; i++)
            {
                params[i].value = i + 1;
                params[i].logSem[0] = &logSem[0];
                params[i].logSem[1] = &logSem[1];
                pthread_create(&threads[i], NULL, thf, &params[i]);
            }
            for (int i = 0; i < 4; i++)
            {
                pthread_join(threads[i], NULL);
            }
            sem_destroy(&logSem[0]);
            sem_destroy(&logSem[1]);
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