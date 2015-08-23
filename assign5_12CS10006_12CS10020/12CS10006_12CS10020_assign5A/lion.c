#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>

int lion_semid;
int jackal_semid;
int ranger_semid;
int empty_semid;
int full_semid;
//int pit_semid;	
int status_semid;
int mutex_semid;

union semun 
	{
        int val;
		struct semid_ds *buf;
		ushort *array;
    } semctlbuf;


struct sembuf sop;

void wait_(int semid,int n,int to)
{
	sop.sem_num=n;
	sop.sem_op=-to;
	sop.sem_flg=0;
	semop(semid,&sop,1);
}
void signal_(int semid,int n,int to)
{
	sop.sem_num=n;
	sop.sem_op=to;
	sop.sem_flg=0;
	semop(semid,&sop,1);
}

void incrsem(int semid,int n,int to)
{
	sop.sem_num=n;
	sop.sem_op=to;
	sop.sem_flg=0;
	semop(semid,&sop,1);	
}

void decrsem(int semid,int n,int to)
{
	sop.sem_num=n;
	sop.sem_op=-to;
	sop.sem_flg=0;
	semop(semid,&sop,1);	
}


int main(int argc, char const *argv[])
{
	
	key_t lion_key;
	key_t jackal_key;
	key_t ranger_key;
	key_t empty_key;
	key_t full_key;
	//key_t pit_key;
	key_t status_key;
	key_t mutex_key;

	
	// key generation to avoide conflicts
	char *path = "/usr";
	lion_key = ftok(path,'P');
	jackal_key = ftok(path,'Q');	
	ranger_key = ftok(path,'R');
	empty_key= ftok(path,'S');
	full_key= ftok(path,'T');
	//pit_key= ftok(path,'U');
	status_key= ftok(path,'V');
	mutex_key= ftok(path,'W');
	
	// create 24 semaphors
	if((lion_semid=semget(lion_key,3,0666|IPC_CREAT))<0) perror("lion_semget:");
	if((jackal_semid=semget(jackal_key,3,0666|IPC_CREAT))<0) perror("jackal_semget:");
	if((ranger_semid=semget(ranger_key,3,0666|IPC_CREAT))<0) perror("ranger_semget:");
	if((empty_semid=semget(empty_key,3,0666|IPC_CREAT))<0) perror("empty_semget:");
	if((full_semid=semget(full_key,3,0666|IPC_CREAT))<0) perror("full_semget:");
	//if((pit_semid=semget(pit_key,3,0666|IPC_CREAT))<0) perror("pit_semget:");
	if((status_semid=semget(status_key,3,0666|IPC_CREAT))<0) perror("status_semget:");
	if((mutex_semid=semget(mutex_key,3,0666|IPC_CREAT))<0) perror("mutex_semget:");
	


	int K;
	printf("enter no. of iteration to be iterated before exiting\n");
	scanf("%d",&K);

	int NL;
	printf("Enter no. of lions\n");
	scanf("%d",&NL);

	int cn;
	for(cn=1;cn<=NL;cn++)
	{
		pid_t pid=fork();
		if(pid==0)
		{
			int i=0;
			srand(time(NULL)/cn/NL);

			for(i=0;i<K;i++)
			{
				// generate random number
				int n=rand()%3;
				printf("%ld Lion %d requesting control over meat pit %d \n",time(0),cn,n+1);

				wait_(mutex_semid,n,1);
				int n_ranger=semctl(ranger_semid,n,GETVAL,semctlbuf);
				int n_jackal=semctl(jackal_semid,n,GETVAL,semctlbuf);	
				int n_pit=semctl(full_semid,n,GETVAL,semctlbuf);

				if(n_ranger==0 && n_jackal==0 && n_pit>0)
				{
				
			 		incrsem(lion_semid,n,1);
			 		decrsem(full_semid,n,1);

					if(semctl(lion_semid,n,GETVAL,semctlbuf)==1)
					{
						signal_(mutex_semid,n,1);
						wait_(status_semid,n,1);
					}
					else
						signal_(mutex_semid,n,1);

							

					printf("%ld Lion %d in control of meat pit %d\n",time(0),cn,n+1);	
					// 	eat
					sleep(5); // sleep for 1ms

					// exit section

					wait_(mutex_semid,n,1);
					decrsem(lion_semid,n,1);
					incrsem(empty_semid,n,1);
					if(semctl(lion_semid,n,GETVAL,semctlbuf)==0)
						signal_(status_semid,n,1);
					signal_(mutex_semid,n,1);

					printf("%ld lion %d done with meat pit %d\n",time(0),cn,n+1);
				}
				else
				{
					printf("%ld Lion %d denied access over meat pit %d\n",time(0),cn,n+1);
					signal_(mutex_semid,n,1); // release last lock

					n=(n+1)%3;

					printf("%ld Lion %d requesting control over meat pit %d \n",time(0),cn,n+1);			
					
					wait_(mutex_semid,n,1);
					int n_ranger=semctl(ranger_semid,n,GETVAL,semctlbuf);
					int n_jackal=semctl(jackal_semid,n,GETVAL,semctlbuf);	
					int n_pit=semctl(full_semid,n,GETVAL,semctlbuf);


					if(n_ranger==0 && n_jackal==0 && n_pit>0)
					{
						incrsem(lion_semid,n,1);
						decrsem(full_semid,n,1);		
						if(semctl(lion_semid,n,GETVAL,semctlbuf)==1)
						{
							signal_(mutex_semid,n,1);
							wait_(status_semid,n,1);
						}
						else
							signal_(mutex_semid,n,1);	

						printf("%ld Lion %d in control of meat pit %d\n",time(0),cn,n+1);	
						// 	eat
						sleep(5); // sleep for 1ms

						wait_(mutex_semid,n,1);
						decrsem(lion_semid,n,1);
						incrsem(empty_semid,n,1);
						if(semctl(lion_semid,n,GETVAL,semctlbuf)==0)
							signal_(status_semid,n,1);
						signal_(mutex_semid,n,1);
						printf("%ld lion %d done with meat pit %d\n",time(0),cn,n+1);
					}
					else
					{
						printf("%ld Lion %d denied access over meat pit %d\n",time(0),cn,n+1);
						signal_(mutex_semid,n,1); // release last lock

						n=(n+1)%3;

						printf("%ld Lion %d requesting control over meat pit %d \n",time(0),cn,n+1);
			 

						wait_(full_semid,n,1);
						wait_(mutex_semid,n,1);
						incrsem(lion_semid,n,1);
						if(semctl(lion_semid,n,GETVAL,semctlbuf)==1)
						{
							//printf("%ld hey baby in lion %d pit %d\n",time(0),cn,n+1);
							signal_(mutex_semid,n,1);
							wait_(status_semid,n,1);
						}
						else
							signal_(mutex_semid,n,1);
						
						
						printf("%ld Lion %d in control over mit pit %d\n",time(0),cn,n+1);
						sleep(5); // sleep for 1ms
						
						wait_(mutex_semid,n,1);
						decrsem(lion_semid,n,1);
						signal_(empty_semid,n,1);
						if(semctl(lion_semid,n,GETVAL,semctlbuf)==0) 
						{
							signal_(status_semid,n,1);
						}

						signal_(mutex_semid,n,1);
						printf("%ld lion %d done with meat pit %d\n",time(0),cn,n+1);
					}
				}
			}
			exit(0);
		}
	}
	int wa=0;
	for(;wa<NL;wa++)
	wait();		
	return 0;
}
