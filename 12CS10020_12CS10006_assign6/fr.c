#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>    
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <utmp.h>
#include <fcntl.h>


int main()
{

        int shmid,f,key=2,i,pid;
        char *ptr;

        shmid=shmget((key_t)key,100,IPC_CREAT|0666);
        ptr=shmat(shmid,NULL,0);
        printf("shmid=%d ptr=%u\n",shmid, ptr);
        pid=fork();
        if(pid==0)
        {
                strcpy(ptr,"hello\n");
        }
        else
        {
                wait(0);
                printf("%s\n",ptr);
        }

}