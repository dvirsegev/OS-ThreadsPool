////// dvir segev
////// 318651627

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <pthread.h>
#include<string.h>
#include "threadPool.h"
#define ERROR "Error in system call"
#define THREADS_COUNT 4
#define TASKS_PER_THREAD 30
#define TASKS_PRI_THREAD 10
#define TP_WORKER_THREADS 3
#define DEBUG 0 // Change to 1 for debug info


void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);
/**
 * @param pool the threadpool
 * @return
 */
static void *threadpool_thread(void *pool) {
    ThreadPool *threadPool = (ThreadPool *) pool;
    if (threadPool == NULL)
        return NULL;
    // check if can run more jobs.
    while(threadPool->run==1) {
        pthread_mutex_lock(&(threadPool->lock));
        // if the queue is empty, "sleep" the thread.
        while (osIsQueueEmpty(threadPool->queue)) {
            pthread_cond_wait(&(threadPool->notify), &(threadPool->lock));
        }
        // create the mission.
        Mission *mission = (Mission *) osDequeue(threadPool->queue);

        /* Wait for element to become available. */

        pthread_mutex_unlock(&(threadPool->lock));

        (*(mission->function))(mission->param);
        pthread_exit(NULL);
    }
}
/**
 * @param numOfThreads how many threads we wnat.
 */


ThreadPool *tpCreate(int numOfThreads) {
    ThreadPool *threadPool = (ThreadPool *) (malloc(sizeof(ThreadPool)));
    if (threadPool == NULL)
        return NULL;
    threadPool->thread_count = numOfThreads;
    // the queue of mission
    threadPool->queue = osCreateQueue();
    // the pool
    threadPool->threads = (pthread_t *) malloc(sizeof(pthread_t) * numOfThreads);
//initialize the newly created threadpool.
    pthread_mutex_init(&(threadPool->lock), NULL);
    pthread_cond_init(&(threadPool->notify), NULL);
    threadPool->run=1;
    threadPool->add=1;


    // create threads
    unsigned int i = 0;
    for (i; i < numOfThreads; i++) {
        if (pthread_create(&(threadPool->threads[i]), NULL,
                           threadpool_thread, threadPool) != 0) {
            tpDestroy(threadPool, 0);
            return NULL;
        }
    }
    return threadPool;
}
/**
 * @param threadPool the threadpool
 * @param computeFunc the function we add
 * @param param the param of the function we add.
 * @return -1 if we cant, 0 if we can.
 */
int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    if(!threadPool->add)
        return -1;
    if (threadPool == NULL || computeFunc == NULL) {
        write(STDERR_FILENO,ERROR, strlen(ERROR));
        return -1;
    }
    Mission *mission = (Mission *) malloc(sizeof(mission));
    mission->function = computeFunc;
    mission->param = param;
    if (pthread_mutex_lock(&(threadPool->lock)) != 0) {
        write(STDERR_FILENO,ERROR, strlen(ERROR));
        return -1;
    }
    osEnqueue(threadPool->queue, mission);
    if (pthread_mutex_unlock(&(threadPool->lock)) != 0) {
        write(STDERR_FILENO,ERROR, strlen(ERROR));
        return -1;
    }
    return 0;
}



void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    int i=0;
    if (pthread_mutex_lock(&(threadPool->lock)) != 0) {
        write(2,ERROR, sizeof(ERROR));
    }
    if (threadPool->add==0) {
        if(pthread_mutex_unlock(&(threadPool->lock))!=0) {
            write(2,ERROR, sizeof(ERROR));
        }
        return;
    }
    threadPool->add=0;
    if(!shouldWaitForTasks) {
        threadPool->run=0;
    }
    if ((pthread_cond_broadcast(&(threadPool->notify)) != 0) ||
        (pthread_mutex_unlock(&(threadPool->lock)) != 0)) {
        write(STDERR_FILENO,ERROR, strlen(ERROR));
    }
    for (i; i < threadPool->thread_count; i++) {
        if (pthread_join((threadPool->threads[i]), NULL) != 0) {
            write(STDERR_FILENO,ERROR, strlen(ERROR));
        }

    }
    threadPool->run=0;
    pthread_mutex_destroy(&(threadPool->lock));
    pthread_cond_destroy(&(threadPool->notify));
    free(threadPool->threads);
    Mission *mission;
    while (mission = (Mission *) osDequeue(threadPool->queue));
    osDestroyQueue(threadPool->queue);
    free(threadPool);
    return;
}
