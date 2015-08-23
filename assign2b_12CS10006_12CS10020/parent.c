#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>

#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500

int main()
{

int i;
//pipe array to store all pipes
int pipe_prnt_chld[5][2];
int pipe_chld_prnt[5][2];

// pid array to store pid of all processe
pid_t pid_array[5];
pid_t pid;

// fork 5 child processes and execvp 5 child processes into them
for(i=0;i<5;i++)
{
	int pipe_prnt_chld_local[2];
	int pipe_chld_prnt_local[2];
	
	if (pipe(pipe_prnt_chld_local) == -1) { perror("pipe"); exit(EXIT_FAILURE);} 
	if (pipe(pipe_chld_prnt_local) == -1) { perror("pipe"); exit(EXIT_FAILURE);}

	pid=fork();
	if(pid<0){ perror("Fork:");}
	if(pid==0)
	{

		    char *argv[6];
            argv[0]=strdup("./child.out");
            char buffer[10];	
            // pass pipe number as arguments
             sprintf(buffer,"%d",pipe_prnt_chld_local[0]);
             argv[1]=strdup(buffer);
             sprintf(buffer,"%d",pipe_prnt_chld_local[1]);
             argv[2]=strdup(buffer);
             sprintf(buffer,"%d",pipe_chld_prnt_local[0]);
             argv[3]=strdup(buffer);
             sprintf(buffer,"%d",pipe_chld_prnt_local[1]);
             argv[4]=strdup(buffer);
             argv[5]=NULL;

            if(execvp("./child.out",argv)<0) perror("failed");
            break;
	}
	else
	{
		// store pid and pipe of current process into array 
		pid_array[i]=pid;
		pipe_prnt_chld[i][0]=pipe_prnt_chld_local[0];
		pipe_prnt_chld[i][1]=pipe_prnt_chld_local[1];
		pipe_chld_prnt[i][0]=pipe_chld_prnt_local[0];
		pipe_chld_prnt[i][1]=pipe_chld_prnt_local[1];
	}	

}

// if parent process
if(pid!=0){

	char *buffer;
	// send id to all child processes
	for(i=0;i<5;i++){
		 close(pipe_prnt_chld[i][0]);          /* Close unused read end */
	     buffer=(char*)malloc(10*sizeof(char));
	     sprintf(buffer,"%d",i+1);
	     write(pipe_prnt_chld[i][1],buffer,strlen(buffer));
	 	 free(buffer);	
	 	}


	int no_ready=0;
	// wait for all childs to send ready signal
	for(i=0;i<5;i++)
	{
		buffer=(char*)malloc(10*sizeof(char));
        char byte = 0;
        int count = 0;

        // close write side. don't need it.
        if(close(pipe_chld_prnt[i][1])!=0) perror("");

        while(read(pipe_chld_prnt[i][0],buffer, sizeof(buffer)) <=0);
        if(atoi(buffer)==READY) no_ready++;
        free(buffer);
    }

    	// all child ready
    	if(no_ready==5)	
    		printf("--- parent READY\n");

  		int k=25/2;
  		srand(time(NULL)); // send seed 

  		while(1)
  		{	
  			int pivot;
  			
  			while(1)
  			{
	  				int rch=rand()%5+1;
					printf("--- parent selected random child %d\n",rch);
					
					buffer=(char*)malloc(10*sizeof(char));
		     		sprintf(buffer,"%d",REQUEST);
		     		write(pipe_prnt_chld[rch-1][1],buffer,strlen(buffer));

		     		free(buffer); 
		     		buffer=(char*)malloc(10*sizeof(char));

					while(read(pipe_chld_prnt[rch-1][0],buffer, sizeof(buffer)) <=0);
        			if(atoi(buffer)==-1) { 
        					printf("--- parent received -1 from child %d \n",rch);
        					free(buffer); 
        					continue;
        				}
        			pivot=atoi(buffer);
        			printf("--- parent received %d from child %d\n",pivot,rch);
        			free(buffer);
        			break;
			}

			// broad cast pivot to all child
			printf("--- Parent broadcasts pivot %d to all children\n",pivot); 
		 	sleep(1);
		 	for(i=0;i<5;i++){
		     buffer=(char*)malloc(10*sizeof(char));
		     sprintf(buffer,"%d",PIVOT);
		     write(pipe_prnt_chld[i][1],buffer,strlen(buffer));
		 	free(buffer);	
		 	}

		 	// broadcast pivot value to all childrens
		 	sleep(1);
		 	for(i=0;i<5;i++){
		     buffer=(char*)malloc(10*sizeof(char));
		     sprintf(buffer,"%d",pivot);
		     write(pipe_prnt_chld[i][1],buffer,strlen(buffer));
		 	free(buffer);	
		 	}

		 	// get number of elements greater than pivot and sum it 
		 	int m=0;
		 	for(i=0;i<5;i++)
		 	{
		    	buffer=(char*)malloc(10*sizeof(char));
				while(read(pipe_chld_prnt[i][0],buffer, sizeof(buffer)) <=0);
				m+=atoi(buffer);
				free(buffer); 
			}

			printf("--- parent received total %d from all child (greater than pivot)\n",m);	

			if(m==k) {
				printf("--- Parent: Median found! Median=%d\n",pivot);
		 		break;
		 	}
		 	if(m>k)
		 	{

				for(i=0;i<5;i++){
			     buffer=(char*)malloc(10*sizeof(char));
			     sprintf(buffer,"%d",SMALL);
			     write(pipe_prnt_chld[i][1],buffer,strlen(buffer));
			 	free(buffer);	
			 	}
		 	}
		 	if(m<k)
		 	{

				for(i=0;i<5;i++){
			     buffer=(char*)malloc(10*sizeof(char));
			     sprintf(buffer,"%d",LARGE);
			     write(pipe_prnt_chld[i][1],buffer,strlen(buffer));
			 	free(buffer);	
			 	}
			 	k=k-m;	
		 	}
		 	sleep(2);
	 	}

	sleep(1);	
    printf("--- Parent sends kill signals to all children \n");	
    for(i=0;i<5;i++)
		if(kill(pid_array[i],SIGUSR1)<0) perror("Kill:");

	int status;
	for(i=0;i<5;i++)
		waitpid(pid_array[i],&status,0);

	printf("--- Parent is going down\n");
}

return 0;

}


