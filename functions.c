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
#include <dirent.h>
#include<sys/stat.h>
#include <sys/dir.h>
#include "work.h"
#include <fcntl.h>
#include "functions.h"

void recurse(char* in,int fd,WorkQueue *queue,int *file_count){
    int count=0;
    char cur[4095];//xrhsimopoieitai gia na swzoume to path
    memset(cur,0,4095);
    struct dirent **fil;
    int entr;
    int i=0;
    entr = scandir(in,&fil,NULL,alphasort);//arithmos twn entries
    //scandir thelw efoson readdir diavazei arxeia me entelws tyxaia seira 
    struct stat s;
    if (entr){//an einai ta entries perissotera apo 0
        while (count<entr){
            if(strcmp(fil[count]->d_name,".")&&strcmp(fil[count]->d_name,"..")){
                //anaparistoun to paron kai to proghoumeno dir, ara den theloume na metaferoume pros apofygh lathwn
                strcpy(cur,in);
                strcat(cur,"/");
                strcat(cur,fil[count]->d_name);
                if(stat(cur,&s)==0){
                    if((s.st_mode&S_IFMT)==S_IFDIR){ //an einai directory
                        recurse(cur,fd,queue,file_count);
                    }
                    else{
                        //prosthese to arxeio sto queue
                        Work *work = init_worker(strdup(cur),fd);
                        flockfile(stdout);
                        printf("[Thread: %ld]: Adding file %s to the queue...\n",pthread_self(),cur);
                        funlockfile(stdout);
                        addWorkQueue(queue,work);
                        (*file_count)++; //ayksise ton synoliko arithmo arxeiwn
                    }
                }

            }
            count++;
        }
    }
    for(int i=0;i<entr;i++){//apodesmeysh mnhmhs efoson to scandir kanei kai malloc
        free(fil[i]);
    }
    free(fil);
}

//metatrepw integer se eidikh morfh se string gia apostolh
char *int_to_string(int num){
    char buf[13]; memset(buf,0,13);
    sprintf(buf,"^%d",num);
    char *msg = strdup(buf);
    return msg;
}

//function gia thread gia na ikanopoihsoume aithmata tou client
void *serve_client(void *arg){
    thread_args *targs = (thread_args*)arg;
    WorkQueue *q = targs->queue;
    int fd = targs->fd;

    char buf[512];
    memset(buf,0,512);

    //paurnw onoma directory apo client
    read(fd,buf,512);
    flockfile(stdout);
    printf("[Thread: %ld]: About to scan directory %s\n",pthread_self(),buf);
    funlockfile(stdout);

    //prosthese arxeia anadromika sto qeue
    int count=0;
    recurse(buf,fd,q,&count);

    char *end_msg = int_to_string(count);//shma katatethen lhkshs prosthikis
    Work *work = init_worker(end_msg,fd);//to eisagw ws arxeio sthn oura
    addWorkQueue(q,work);
}

//Function gia na grapsw sto socket dedomenou block size wste na eimaste asfaleis
void socketWrite(int fd,int bufferSize,const char *msg){

    char msg_value[12]; memset(msg_value,0,12);
    sprintf(msg_value,"^%d^",(int)strlen(msg));

    //an to mhnyma megalytero tou block size
    //tote to spame se kommatia
    if((strlen(msg)+strlen(msg_value))>bufferSize) {
        // to katallhlo mhkos einai to arxiko string dia
        //buf size - 14(afou 12 gia int kai 2 gia ta ^) 
        char total_output[bufferSize];
        int msg_num = strlen(msg)/(bufferSize-14);//arithmos kommatiwn
        if(strlen(msg)%(bufferSize-14)!=0)
            msg_num++;
        int msg_len = strlen(msg)/msg_num;
        if(strlen(msg)%msg_num!=0)
            msg_len++;
        for(int i=0;i<msg_num;i++) {//gia kathe ena apo ta kommatia

            char divided_msg[msg_len+1];
            memset(msg_value, 0, 12);
            memset(divided_msg,0,msg_len+1);
            memset(total_output,0,bufferSize);

            //an eimaste sto teleytaio kommati kai to ypoloipomeno megethos den einai akribws to provlepomeno
            if(i==msg_num-1 && strlen(msg)%msg_num!=0)
                memcpy(divided_msg,msg+i*(msg_len),(strlen(msg)-i*msg_len));
            else
                memcpy(divided_msg,msg+i*(msg_len),msg_len);

            //neo synoliko mhkos tou kommenou mhnymatos
            sprintf(msg_value, "^%d^", (int)strlen(divided_msg));
            strcpy(total_output,msg_value);
            strcat(total_output,divided_msg);
            //an einai teleytaio iteration
            //symvolo # gia na deiksw oti teleiwse to mhnyma
            if(i==msg_num-1)
                strcat(total_output,"#");

            write(fd,total_output,strlen(total_output)+1);
        }
    }
        //an mhnyma mikrotero tou block size tote grafetai oloklhro xwris provlhma
    else{
        char total_output[bufferSize];
        memset(total_output,0,bufferSize);
        strcpy(total_output,msg_value);
        strcat(total_output,msg);
        strcat(total_output,"#");

        //to grafw sto sock
        write(fd,total_output,strlen(total_output)+1);
    }
}

//Function na diavazw apto socket dedomenou to block size
char *socketRead(int fd){

    char *msg=malloc(1);
    char *total_input=NULL;
    memset(msg,0,1);
    char cur_value[12];
    char msg_len[12];
    char msg_end[12];
    int msg_size;
    while(1){
        memset(cur_value,0,12);
        memset(msg_len,0,12);
        memset(msg_end,0,12);
        //diavazw mexri na vrw to prwto ^
        read(fd,cur_value,1);
        while(strcmp(cur_value,"^")!=0) read(fd,cur_value,1);
        memset(cur_value,0,12);
        //arxizw na diavazw to megethos
        read(fd, cur_value, 1);
        while (strcmp(cur_value,"^") != 0) {
            strcat(msg_len, cur_value);
            read(fd, cur_value, 1);
        }
        msg_size = atoi(msg_len);//ton metatrepw se integer afou einai se pinaka char
        total_input = malloc(msg_size+1);
        memset(total_input,0,msg_size+1);
        read(fd,total_input,msg_size);
        read(fd,msg_end,1);
        msg = realloc(msg,strlen(msg)+strlen(total_input)+2);
        strcat(msg,total_input);
        free(total_input);
        total_input=NULL;
        //an o xarakthras # tote stamatw afou exei erthei to telos
        if(msg_end[0]=='#')
            break;
        else
            strcat(msg,msg_end);//alliws kollaw ayto pou einai to teleytaio gramma
    }
    //clear to buffer
    read(fd,cur_value,1);
    return msg;
}

void cre(char *path) {
    char *inside;
    if(path[0]=='/'){ //an mas dothei path se morfh /home/users
        inside=path+1; //to skipparoume
    }
    else{
        inside=path; //aliws to agnoooume
    }
    int arx=0,mes=0,tel=0;
    char *msg=strdup(inside);
    char *start=NULL;
    char *siz=NULL;
    char *cont=NULL;
    start = strtok(msg,"@@"); //efoson kseroume oti einai tria "kommatia",  aplws kaloume treis fores thn strtok
    siz=strtok(NULL,"@@");
    int si=atoi(siz);
    cont=strtok(NULL,"@@");

//spasimo tou path poy exoume se ayto mono me dir(gia mkdir) kai se oloklhro(gia open)
    char whole[300];
    memset(whole,0,300);
    strcpy(whole,msg); //edw krataw to oloklhro gia to open
    //printf("whole is:%s\n",whole);
    char *sta=NULL;
    sta=strrchr(msg,'/');//vriskw thn teleytaia emfanish tou h opoia einai akrivws prin to onoma tou arxeiou
    //printf("%s\n",sta);
    *sta='\0'; //ekei pleon stamataw to string wste na exw apokleistika to dir
    char odir[300];
    strcpy(odir,msg); //edw krataw apokleistika ta dir
    //printf("onlydir is:%s\n",odir);
    //free(msg);
    char nam[300];
    memset(nam,0,300);
    strcpy(nam,odir);
    char *st=NULL;
    char usefmk[300];
    memset(usefmk,0,300);
    st=strtok(nam,"/");
    while(st!=NULL){
        //printf("%s\n",st);
        strcat(usefmk,st);
        strcat(usefmk,"/");
        //printf("usefmk:%s\n",usefmk);
        mkdir(usefmk,0777);
        st=strtok(NULL,"/");
    }

    struct stat s;
    if((stat(whole,&s))==0){
        printf("file exists.\n");
        unlink(whole);//svisimo tou arxeiou se periptwsh poy proyphrxe
        printf("file has been removed as it existed earlier.\n");
    }
    int filedes=open(whole,O_RDWR|O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG);
    if(filedes==-1){
        perror("creation");
        exit(-4);
    }
    write(filedes,cont,si);
    //write(filedes,"\0",1);
    close(filedes);
    free(msg);
}
