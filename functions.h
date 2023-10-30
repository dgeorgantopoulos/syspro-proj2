#ifndef THREAD_POOL_FUNCTIONS_H
#define THREAD_POOL_FUNCTIONS_H

#include "work.h"

typedef struct thread_args{
    WorkQueue *queue;
    int fd;
}thread_args;

void *serve_client(void *);
void recurse(char* ,int ,WorkQueue *,int *);
void socketWrite(int,int,const char *);
char *socketRead(int);
char *int_to_string(int num);
void cre(char*);

#endif //THREAD_POOL_FUNCTIONS_H
