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
#include "functions.h"



int main(int argc,char *argv[]){
	struct sockaddr_in server;
	int sock;
	struct sockaddr * serverptr=(struct sockaddr *) &server;
	char ip[20];
	strcpy(ip,argv[2]);
	int port=atoi(argv[4]);
	struct hostent *rem;
	if (( sock = socket ( AF_INET , SOCK_STREAM , 0) ) < 0)//efoson tcp
		perror( " socket ") ;
	server.sin_family = AF_INET ;
	server.sin_addr.s_addr=inet_addr(ip);
	server.sin_port = htons(port);
	if ( connect ( sock , serverptr , sizeof ( server ) ) < 0)
		perror("connect");
	//printf ( " Connecting to %s port %d\n " , argv[1] , port ) ;
	write(sock,argv[6],strlen(argv[6]));

    int count=0;
    int files=-1;
    while(1) {
        char *msg = socketRead(sock);
        count++;
        //printf("\n----------\n%s\n-----------\n", msg);
        if(msg[0]=='^'){
            files=atoi(msg+1);
        }
        else{
        	cre(msg);//synarthsh gia dhmiourgia arxeiwn- tyxon directories
        }
        //diavase ola ta arxeia - stamata
        free(msg);
        if(count-1==files)
            break;
    }

    close(sock);
    printf("Directory copied from server...\n");
    return 0;
}
