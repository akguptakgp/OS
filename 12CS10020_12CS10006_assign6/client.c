#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>	
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/shm.h>

#define msgsize 500 
const int maxclinet=100;

typedef struct
 {
    long mtype;       /* message type, must be > 0 */
    char mtext[msgsize];    /* message data */
}msgbuf;


msgbuf message; 

int shm1_id;
int shm2_id;
int sem1_id;
int sem2_id;
int mq_id;
char msg[200]=".";

void down(int semid,int to)
{
  struct sembuf sop;
  sop.sem_num=0;
  sop.sem_op=-to;
  sop.sem_flg=0;
  semop(semid,&sop,1);
}
void up(int semid,int to)
{ 
  struct sembuf sop;
  sop.sem_num=0;
  sop.sem_op=to;
  sop.sem_flg=0;
  semop(semid,&sop,1);
}

void parenthander()
{
    printf("%ld <Ctrl+C is pressed>\n--- Enter your message: \n",time(0));
   // printf("semphore %d \n",semctl(sem2_id,0,GETVAL,0));
    gets(msg);
    //printf("dddddddddddddddddddd %s\n",msg);
}

void childhander()
{
  return;
}


void commomhandler()
{
  exit(0);
}

int main()
{
      signal(SIGSTOP,commomhandler);
      if( access("ser.txt", F_OK ) == -1 )
      {
        printf("server not running\n");
        exit(0);
      }
       
      key_t sem1_key;
      key_t sem2_key;
      key_t shm1_key;
      key_t shm2_key;
      key_t mq_key;

      char *path = "/usr";
      sem1_key=ftok(path,'P');
      sem2_key =ftok(path,'Q');
      shm1_key=ftok(path,'R');
      shm2_key =ftok(path,'S');
      mq_key = ftok(path,'T');


      //=ftok(path, 'S');
      if((mq_id=msgget(mq_key,IPC_CREAT|0666))<0) perror("message queue create:");
      if((sem1_id=semget(sem1_key,1,0666|IPC_CREAT))<0) perror("sem1_semget:");
      if((sem2_id=semget(sem2_key,1,0666|IPC_CREAT))<0) perror("sem2_semget:");
      if((shm1_id=shmget(shm1_key,500,0666|IPC_CREAT))<0) perror("shm1_semget:");
      if((shm2_id=shmget(shm2_key,5000,0666|IPC_CREAT))<0) perror("shm2_semget:");


      char buffer[100]; 
      pid_t *PIDArr;
      char *Message;

      PIDArr=(pid_t *)shmat(shm1_id,NULL,0);
      Message=(char*)shmat(shm2_id,NULL,0);

      char *login;
      char *sp_msg=".";
      char *sp_ter="*";

     //printf("--- Initialization complete client pid =%d\n",getpid());
     
      char str[15];
      pid_t pid;
      int i=1;

        pid=fork();
        if(pid==-1)
        {
           perror("fork error");
        }
        if(pid==0)
        {

            signal(SIGINT,childhander);
            printf("--- Initialization complete receiver pid =%d\n",getpid());
            down(sem1_id,1);
            PIDArr[0]=PIDArr[0]+1;
            PIDArr[PIDArr[0]]=getppid();
            up(sem1_id,1);

            while(1)
            {
              
              int size=msgrcv(mq_id,&message,sizeof(message.mtext),getppid()%maxclinet+1,0);
            
              if(size>0) 
              { 
                printf("--- Received message: \"%s\"\n",message.mtext);
               // puts(message.mtext);
              }

            }
         }   
        else
        {   
              int flg=0;
              sleep(3);
              signal(SIGINT,parenthander);
              printf("--- Initialization complete sender pid =%d\n",getpid());
              while(1)
              { 
                printf("---  sender for sleeping for %d secs\n",5);
                sleep(5);
                printf("---  sender sleep done\n");
                  if(!strcmp("bye",msg))
                  {
                      int i=1;
                      down(sem1_id,1);
                      for(i=1;i<=PIDArr[0];i++)
                      {
                        if(PIDArr[i]==getpid()) 
                        {
                            int j=i+1;
                            for(j=i+1;j<=PIDArr[0];j++)
                            {
                              PIDArr[j-1]=PIDArr[j];
                            }
                        }
                      }  
                      PIDArr[0]=PIDArr[0]-1;
                      if(PIDArr[0]==0)
                         strcpy(msg,sp_ter); 
                      up(sem1_id,1);
                      flg=1;
                    } 
                  
                    login=getlogin(); 
                    // down(sem2_id,0);
                    // sleep(3);
                    // up(sem2_id,1);

                    // atomic two operation
                    //printf("sem2 =%d\n",semctl(sem2_id,0,GETVAL,0));
                    struct sembuf sop[2];
                    sop[0].sem_num=0;
                    sop[0].sem_op=0;
                    sop[0].sem_flg=0;
                    sop[1].sem_num=0;
                    sop[1].sem_op=1;
                    sop[1].sem_flg=0;
                    semop(sem2_id,sop,2);
                    //printf("%ld  client in control\n",time(0));
                    //sleep(3);
                    //printf("sem2 =%d\n",semctl(sem2_id,0,GETVAL,0));
                    printf("---  sending msg \"%s/%d:%s\"\n",login,getpid(),msg);
                    sprintf(Message,"%s/%d:%s",login,getpid(),msg);  
                    strcpy(msg,sp_msg);
                   // sleep(3);
                   // up(sem2_id,1);  
                    semctl(sem2_id,0,SETVAL,2);

                    //printf("%ld client leave\n",time(0));
                   // printf("sem2 =%d\n",semctl(sem2_id,0,GETVAL,0));
                  if(flg)
                  {
                    kill(pid,SIGSTOP);
                    exit(0);
                  }

              }         
        }

}