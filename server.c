#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include "work.h"
#include "functions.h"

int main(int argc,char *argv[]){
    //Command line arguments
    int THREAD_POOL_SIZE = atoi(argv[4]);
    int QUEUE_SIZE = atoi(argv[6]);

	int opt=1;
	socklen_t optlen=sizeof(opt);
	int port=atoi(argv[2]),sock,newsock;
	int block_size=atoi(argv[8]);

    //arxikopoihsh thread pool kai queue
    pthread_t t[THREAD_POOL_SIZE];
    WorkQueue *queue = initWorkQueue(QUEUE_SIZE,block_size);

    for(int i=0;i<THREAD_POOL_SIZE;i++)//dhmiourghse  ola ta worker threads eksarxhs me starting routine thn worker thread
        pthread_create(&t[i], NULL, worker_thread, queue);

    printf("Server’s parameters are:\n");
    printf("port: %d\n",port);
    printf("thread_pool_size: %d\n",THREAD_POOL_SIZE);
    printf("queue_size: %d\n",QUEUE_SIZE);
    printf("Block_size: %d\n",block_size);

	struct sockaddr_in server,client;
	socklen_t clientlen = sizeof (client);
	struct sockaddr * serverptr=(struct sockaddr *) &server ;
	struct sockaddr * clientptr=(struct sockaddr *) &client ;
	if ((sock=socket(AF_INET,SOCK_STREAM, 0) ) == -1){
		perror ( " Socket creation failed ! ");
	}
	server.sin_family = AF_INET ; 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port); //to port ths grammhs entolwn
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,optlen);//gia  apofygh errors
	if( bind (sock ,serverptr ,sizeof(server) ) < 0){
		perror("bind");
	}
	if(listen(sock,5)< 0){ 
		perror("listen");
	}

    printf("Server was successfully initialized...\n");
    printf("Listening for connections to port %d\n",port);

	while (1){
		if((newsock = accept(sock ,clientptr, &clientlen ))< 0){ 
			perror("accept");
		}
        printf("\nAccepted connection\n\n");
        pthread_t com_t;
        thread_args *args = malloc(sizeof(thread_args));
        args->queue = queue;
        args->fd = newsock;

        //dhmiourgia comm thread gia neo client
        pthread_create(&com_t, NULL, serve_client, args);
	}
}
