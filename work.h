#ifndef THREAD_POOL_WORK_H
#define THREAD_POOL_WORK_H

#include <pthread.h>

typedef struct work_queue WorkQueue;

typedef void (*workerFunction)(WorkQueue *,char *,int);

typedef struct work{
    char *filename;
    int fd;
    workerFunction fun;
}Work;

Work *init_worker(char *file,int fd);

typedef struct work_queue{
    int size;
    int front;
    int rear;
    pthread_mutex_t  work_mutex;
    pthread_cond_t   work_cond;
    pthread_mutex_t  write_mutex;
    pthread_cond_t   write_cond;
    Work **queue;
    int block_size;
}WorkQueue;

void *worker_thread(void *arg);

void parse_file(WorkQueue *q,char *path,int fd);

WorkQueue *initWorkQueue(int qsize,int block_size);

WorkQueue *addWorkQueue(WorkQueue *queue,Work *work);

int fullWorkQueue(WorkQueue *queue);

int emptyWorkQueue(WorkQueue *queue);

#endif //THREAD_POOL_WORK_H
