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

typedef struct {
    long mtype;       /* message type, must be > 0 */
    char mtext[msgsize];    /* message data */
}msgbuf;


int shm1_id;
int shm2_id;
int sem1_id;
int sem2_id;
int mq_id;

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



int main(int argc, char const *argv[])
{
	key_t sem1_key;
	key_t sem2_key;
	key_t shm1_key;
	key_t shm2_key;
    key_t mq_key;
	
    char buffer[100];	

    pid_t *PIDArr;
    char *Message;

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

	// init part
	semctl(sem1_id,0,SETVAL,1);
	semctl(sem2_id,0,SETVAL,0);
	PIDArr=(pid_t *)shmat(shm1_id,NULL,0);
	Message=(char*)shmat(shm2_id,NULL,0);
	memset(Message,'\0',5000);
	memset(PIDArr,'\0',500);
	PIDArr[0]=0;

	printf("--- Initialization complete server pid =%d\n",getpid());

	int no_args=argc-1;
	int i=1;
	
	
	int crt=open("ser.txt", O_CREAT|O_EXCL|O_WRONLY,S_IRWXU);
	if(crt==-1)
	{
		perror("serveropen");
		exit(0);
	}
	else
	{
		sprintf(buffer,"%d",getpid());
		write(crt,buffer,strlen(buffer));
	}


	

	FILE *fp=fopen("/var/run/utmp","r");
	if (fp==NULL) { fputs ("File error",stderr); exit (1);}
	
	struct utmp u; 
	int flag=0;
	int cnt=0;
	while(fread(&u, sizeof(u), 1, fp) >0)
	{	
		flag=0;
		if(u.ut_name!=NULL && getpwnam(u.ut_name)!=NULL)
		{
			
			for (i = 1; i <=no_args; ++i)
			{
				if(!strcmp(argv[i],u.ut_line)) flag=1;
			}

			if(cnt++!=0 && getpid()-u.ut_pid<2000 && u.ut_type!=DEAD_PROCESS && flag)// && strcmp(u.ut_line,"pts/1"))
			{
				// printf("%d %d %d\n",u.ut_type,u.ut_pid,getpid());
				char name[50];
				sprintf(name,"./commence.out > /dev/%s", u.ut_line);
				//printf("%s\n",name);
				printf("--- Sending commence notification to line=%s user=%s ter=%s\n",u.ut_line,u.ut_name,u.ut_id);
				if(system(name)<0);// perror("s");
				sleep(1);
			}
		}
	}
	fclose (fp);

	msgbuf message;
	char * pch;
	char *str;
    char *sp_msg=".";
    char *sp_ter="*";
 


	char user[200];
	pid_t pid;
	char msg[500];


	// printf("%s %s\n",sp_msg,sp_ter);

	while(1)
	{
		//sleep(3);

		//printf("sem2 =%d\n",semctl(sem2_id,0,GETVAL,0));
		struct sembuf sop[2];
        sop[0].sem_num=0;
        sop[0].sem_op=-2;
        sop[0].sem_flg=0;
        sop[1].sem_num=0;
        sop[1].sem_op=2;
        sop[1].sem_flg=0;
        semop(sem2_id,sop,2);
       // printf("sem2 =%d\n",semctl(sem2_id,0,GETVAL,0));
        //printf("%ld server in control %d\n",time(0),semctl(sem2_id,0,GETVAL,0));
     	 //sleep(3);
		str=strdup(Message);
		memset(Message,'\0',5000);
		//sleep(1);;
		down(sem2_id,2);	
		//printf("%ld server leaves %d\n",time(0),semctl(sem2_id,0,GETVAL,0));	
		//printf("sem2 =%d\n",semctl(sem2_id,0,GETVAL,0));
		printf("--- Received msg \"%s\"\n",str);
		// puts(str);

		int count=0;
		char * pch;
		pch = strtok(str,"/:");
		while(pch!='\0')
		{
			// printf ("%s\n",pch);
			if(count==0){
			strcpy(user,pch);
			// printf("user:%s\n",user);
			}
			else if(count==1){
				
				pid=atoi(pch);
				// printf("pid: %d\n",pid);
			}
			else{
				
			strcpy(msg,pch);
			// printf("msg: %s\n",msg);
			}
			count++;
		    pch = strtok (NULL, "/:");
		}
		//printf("%ld message decoded %s %d %s\n",time(0),user,pid,msg);

		if(!strcmp(msg,sp_msg))
		{
			// printf("Special mesaage");
		}
		else if(!strcmp(msg,sp_ter)) //         termination
		{
			printf("--- Terminating conference.");
			if(semctl(sem1_id,1,IPC_RMID,1)<0)  perror("sem1 rm:");
			if(semctl(sem2_id,1,IPC_RMID,1)<0) perror("sem2 rm");
			if(shmdt(PIDArr)<0)  perror("shm1 rm:");
			if(shmdt(Message)<0) perror("shm2 rm");
			if(msgctl(mq_id,IPC_RMID,NULL)<0) perror("message queue remove");
			remove("ser.txt");
			exit(1);
		}
		else // broadcast 
		{
			// sleep(3);
			int i=1;
			for(i=1;i<=PIDArr[0];i++)
			{
				if(PIDArr[i]!=pid)
				{
					printf("---  sending msg to pid %d\n",PIDArr[i]);
					message.mtype= PIDArr[i]%maxclinet+1;
				 	sprintf(message.mtext,"%s/%d:%s",user,pid,msg);
				 	//printf("%ld message=%s\n",time(0),message.mtext);
				 	int s=msgsnd(mq_id,&message,sizeof(message.mtext),0);
				 	if(s<0) perror("server msgsnd");
				 	
				}
			}
		}
		sleep(5);				
	}
	return 0;

}