////// dvir segev
////// 318651627

#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include "osqueue.h"
#include <pthread.h>
typedef struct thread_pool
{
    //The field x is here because a struct without fields
    //doesn't compile. Remove it once you add fields of your own
    int thread_count;
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    OSQueue *queue;
    int run;
    int add;
    //TODO - FILL THIS WITH YOUR FIELDS
}ThreadPool;

typedef struct misssion
{
    void (*function)(void *);
    void * param;
}Mission;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);
static void *threadpool_thread(void *pool);
#endif