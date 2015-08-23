#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <signal.h>
#include <time.h>

int customer_semid;
int barber_semid;
int mutex_semid;
int waiting_semid;

union semun 
	{
        int val;
		struct semid_ds *buf;
		ushort *array;
    } sembuf;


struct sembuf sop;
void down(int semid)
{
	sop.sem_num=0;
	sop.sem_op=-1;
	sop.sem_flg=0;
	semop(semid,&sop,1);
}
void up(int semid)
{
	sop.sem_num=0;
	sop.sem_op=1;
	sop.sem_flg=0;
	semop(semid,&sop,1);
}

int main(int argc, char const *argv[])
{
	// signal(SIGINT,exithander);
	int n,i,cust;
	key_t customer_key;
	key_t barber_key;
	key_t mutex_key;
	key_t waiting_key;

	// key generation to avoide conflicts
	char *path = "/usr";
	customer_key = ftok(path,'P');
	barber_key = ftok(path,'Q');	
	mutex_key = ftok(path,'R');
	waiting_key=ftok(path,'S');
	
	// create 3 semaphors
	if((customer_semid=semget(customer_key,1,0666|IPC_CREAT))<0) perror("customer_semget:");
	if((barber_semid=semget(barber_key,1,0666|IPC_CREAT))<0) perror("barber_semget:");
	if((mutex_semid=semget(mutex_key,1,0666|IPC_CREAT))<0) perror("mutex_semget:");
	if ((waiting_semid=semget(waiting_key,1,0666|IPC_CREAT))<0) perror("waiting_semget:");
	
     printf("Enter number of chairs\n");
     scanf("%d",&n);

	printf("Enter number of customers\n");
     scanf("%d",&cust);

    for(i=0;i<cust;i++)
    {
     	pid_t  pid;
     	pid = fork();
     	if (pid == 0)
     	{	

     		printf("customer %d Reqesting for hair cut\n",i+1);
 			down(mutex_semid);
 			if(semctl(waiting_semid,0,GETVAL,0)<n)
 			{
 				up(waiting_semid);
 				up(customer_semid);
 				up(mutex_semid);
 				down(barber_semid);
 				printf("barber in control of customer %d\n",i+1);
 				int r = rand() % 3;
				sleep(r);
				printf("hair cut of customer %d done\n",i+1);

 			}
			else
			{
				printf("customer %d denied access to barber shop leaving the shop...\n",i+1);
				up(mutex_semid);
			}
			exit(0);
		} 

     }
     for(i=0;i<cust;i++)
     	wait();

	return 0;
}

