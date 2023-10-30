#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "work.h"
#include "functions.h"

char *pack_file_message(char *path){
    //pairnw to onoma tou file
    char *msg = strdup(path);

    //megethos tou file
    struct stat s;
    stat(path,&s);
    int file_size = s.st_size;
    char fsize[12]; memset(fsize,0,12);
    sprintf(fsize,"%d",file_size);
    msg = realloc(msg,strlen(msg)+strlen(fsize)+5+file_size);
    strcat(msg,"@@");
    strcat(msg,fsize);
    strcat(msg,"@@");

    //ta contents tou
    char buf[file_size+1];memset(buf,0,file_size+1);
    int fd = open(path,O_RDONLY);
    read(fd,buf,file_size);
    strcat(msg,buf);
    close(fd);

    return msg;
}

void parse_file(WorkQueue *q,char *path,int fd){

    if(path[0]=='^'){
        pthread_mutex_lock(&(q->write_mutex));
        socketWrite(fd,q->block_size,path);
        pthread_mutex_unlock(&(q->write_mutex));
        return;
    }

    flockfile(stdout);
    printf("[Thread: %ld]: About to read file %s\n",pthread_self(),path);
    funlockfile(stdout);

    //Package the file as a message
    char *msg = pack_file_message(path);

    //grafw to message ston client
    pthread_mutex_lock(&(q->write_mutex));
    socketWrite(fd,q->block_size,msg);
    free(msg);
    pthread_mutex_unlock(&(q->write_mutex));
}

Work *init_worker(char *file,int fd){
    Work *work = malloc(sizeof(Work));
    work->filename = file;
    work->fd = fd;
    work->fun = parse_file;
    return work;
}

//initialisation gia to queue
WorkQueue *initWorkQueue(int qsize,int block_size){
    WorkQueue *q = malloc(sizeof(WorkQueue));
    q->block_size = block_size;
    q->size=qsize;
    q->front=0;
    q->rear=0;
    pthread_mutex_init(&(q->work_mutex), NULL);
    pthread_cond_init(&(q->work_cond), NULL);
    pthread_mutex_init(&(q->write_mutex), NULL);
    pthread_cond_init(&(q->write_cond), NULL);
    q->queue = malloc(sizeof(Work*)*qsize);
    for(int i=0;i<qsize;i++) q->queue[i]=NULL;
    return q;
}

//chekarw an queue gemato
int fullWorkQueue(WorkQueue *queue){
    return (queue->rear == queue->size);
}

//an queue adeio
int emptyWorkQueue(WorkQueue *queue){
    return (queue->front == queue->rear);
}

//prosthiki arxeiou-ergasia stio queue
WorkQueue *addWorkQueue(WorkQueue *queue,Work *work){
    pthread_mutex_lock(&(queue->work_mutex));

    while(fullWorkQueue(queue))
        pthread_cond_wait(&(queue->write_cond),&(queue->work_mutex));

    queue->queue[queue->rear]=work;
    queue->rear++;

    pthread_cond_broadcast(&(queue->work_cond));
    pthread_mutex_unlock(&(queue->work_mutex));
    return queue;
}

//apoxwrhsh douleias apo queue
Work *popWorkQueue(WorkQueue *queue){
    if(emptyWorkQueue(queue)==1){
        return NULL;
    }
    else{
        Work *top = queue->queue[queue->front];
        queue->front++;
        if(queue->front==queue->rear){
            queue->front=0;
            queue->rear=0;
        }
        return top;
    }
}
//synarthsh pou tha ektelei kathe worker thread kata thn dhmioyrgia ths
void *worker_thread(void *arg){
    WorkQueue *q = arg;
    Work *work;

    while (1) {
        pthread_mutex_lock(&(q->work_mutex));

        while (emptyWorkQueue(q))
            pthread_cond_wait(&(q->work_cond), &(q->work_mutex));

        work = popWorkQueue(q);

        flockfile(stdout);
        if(strcmp(work->filename,"END")!=0)
            printf("[Thread: %ld]: Received task <%s,%d>\n",pthread_self(),work->filename,work->fd);
        funlockfile(stdout);

        pthread_cond_signal(&(q->write_cond));
        pthread_mutex_unlock(&(q->work_mutex));

        if (work != NULL) {
            work->fun(q,work->filename,work->fd);
            free(work->filename);
            free(work);
        }
    }

    return NULL;
}
