#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>	
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#define CHAIRS 5

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
	//signal(SIGINT,exithander);
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
	if((waiting_semid=semget(waiting_key,1,0666|IPC_CREAT))<0) perror("waiting_semget:");

	while(1)
	{

		srand(time(NULL));
		printf("barber waiting for customers\n");
		down(customer_semid);
		down(mutex_semid);
		down(waiting_semid);
		up(barber_semid);
		up(mutex_semid);
		int r = rand() % 3;
		sleep(r);
		printf("cutting done\n");
	}

	return 0;
}




