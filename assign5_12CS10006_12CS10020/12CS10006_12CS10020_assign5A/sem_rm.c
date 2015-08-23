#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

int lion_semid;
int jackal_semid;
int ranger_semid;
int empty_semid;
int full_semid;
int pit_semid;	
int status_semid;
int mutex_semid;

union semun 
	{
        int val;
		struct semid_ds *buf;
		ushort *array;
    } semctlbuf;


int main(int argc, char const *argv[])
{
	key_t lion_key;
	key_t jackal_key;
	key_t ranger_key;
	key_t empty_key;
	key_t full_key;
	key_t pit_key;
	key_t status_key;
	key_t mutex_key;

	
	// key generation to avoide conflicts
	char *path = "/usr";
	lion_key = ftok(path,'P');
	jackal_key = ftok(path,'Q');	
	ranger_key = ftok(path,'R');
	empty_key= ftok(path,'S');
	full_key= ftok(path,'T');
	pit_key= ftok(path,'U');
	status_key= ftok(path,'V');
	mutex_key= ftok(path,'W');
	
	// create 24 semaphors
	if((lion_semid=semget(lion_key,3,0666|IPC_CREAT))<0) perror("lion_semget:");
	if((jackal_semid=semget(jackal_key,3,0666|IPC_CREAT))<0) perror("jackal_semget:");
	if((ranger_semid=semget(ranger_key,3,0666|IPC_CREAT))<0) perror("ranger_semget:");
	if((empty_semid=semget(empty_key,3,0666|IPC_CREAT))<0) perror("empty_semget:");
	if((full_semid=semget(full_key,3,0666|IPC_CREAT))<0) perror("full_semget:");
	if((pit_semid=semget(pit_key,3,0666|IPC_CREAT))<0) perror("pit_semget:");
	if((status_semid=semget(status_key,3,0666|IPC_CREAT))<0) perror("status_semget:");
	if((mutex_semid=semget(mutex_key,3,0666|IPC_CREAT))<0) perror("mutex_semget:");
	

// remove semaphores
	if(semctl(lion_semid,3,IPC_RMID,semctlbuf)<0)  perror("lion rm:");
	if(semctl(jackal_semid,3,IPC_RMID,semctlbuf)<0) 	perror("jackal rm:");
	if(semctl(ranger_semid,3,IPC_RMID,semctlbuf)<0)  perror("ranger rm:");
	if(semctl(empty_semid,3,IPC_RMID,semctlbuf)<0) 	perror("empty rm:");
	if(semctl(full_semid,3,IPC_RMID,semctlbuf)<0) 	perror("full rm:");
	if(semctl(pit_semid,3,IPC_RMID,semctlbuf)<0) 	perror("pitrm:");
	if(semctl(status_semid,3,IPC_RMID,semctlbuf)<0)  perror("status rm:");
	if(semctl(mutex_semid,3,IPC_RMID,semctlbuf)<0) 	perror("mutex rm:");

	return 0;
}
