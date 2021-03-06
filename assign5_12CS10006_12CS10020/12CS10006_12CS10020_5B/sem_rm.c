#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

int customer_semid;
int barber_semid;
int mutex_semid;
int waiting_semid ;


union semun 
	{
        int val;
		struct semid_ds *buf;
		ushort *array;
    } semctlbuf;


int main(int argc, char const *argv[])
{
	
  
	key_t customer_key;
	key_t barber_key;
	key_t mutex_key;
	key_t waiting_key; 
	
	// key generation to avoide conflicts
	char *path = "/usr";
	customer_key=ftok(path,'P');
	barber_key =ftok(path,'Q');
	mutex_key =ftok(path,'R');
	waiting_key =ftok(path,'S');
	
	if((customer_semid=semget(customer_key,1,0666|IPC_CREAT))<0) perror("customer_semget:");
	if((barber_semid=semget(barber_key,1,0666|IPC_CREAT))<0) perror("barber_semget:");
	if((mutex_semid=semget(mutex_key,1,0666|IPC_CREAT))<0) perror("mutex_semget:");
	if((waiting_semid=semget(waiting_key,1,0666|IPC_CREAT))<0) perror("waiting_semget:");



// remove semaphores

	if(semctl(customer_semid,1,IPC_RMID,semctlbuf)<0)  perror("customer rm:");
	if(semctl(barber_semid,1,IPC_RMID,semctlbuf)<0)  perror("barber rm:");
	if(semctl(mutex_semid,1,IPC_RMID,semctlbuf)<0)  perror("mutex rm:");
	if(semctl(waiting_semid,1,IPC_RMID,semctlbuf)<0)  perror("wait rm:");
	return 0;
}
