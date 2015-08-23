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

int ID;

void childSigHandler_id(int sig)
{ 
	printf("--- Child %d got parent's kill signal exiting\n",ID);
	exit(0);
}

int main(int argc,char *argv[])
{
	// register signal
	signal(SIGUSR1,childSigHandler_id);

	//declare and receive pipe
	int pipe_prnt_chld[2];
	int pipe_chld_prnt[2];
	pipe_prnt_chld[0]=atoi(argv[1]);
	pipe_prnt_chld[1]=atoi(argv[2]);
	pipe_chld_prnt[0]=atoi(argv[3]);
	pipe_chld_prnt[1]=atoi(argv[4]);

	// will be used to communicate through pipe
	char buffer[10];
	
	//close write side. don't need it.
	close(pipe_prnt_chld[1]);

	// read ID
	while(read(pipe_prnt_chld[0], buffer, sizeof(buffer)) <=0);
	ID=atoi(buffer);
	       
	// read file data_i.txt
	char file_name[20];
	sprintf(file_name,"data_%d.txt",ID);
	FILE *f=fopen(file_name,"r");
	if(f==NULL) printf("ERROR\n");
	int array[5];
	int size=5;
	int i=0;
	char ch;
	for(i=0;i<size;i++) array[i]=0;
	i=0;	
	while(( ch = fgetc(f)) != EOF)
      {
      	if(ch!=' ')
        	array[i]=array[i]*10+ch-'0';
        else
        	i++;
     }
	 fclose(f);

	// send ready signal to parent
	printf("--- Child %d sends READY\n",ID);
	close(pipe_chld_prnt[0]);          /* Close unused read end */
	char msg[20];
	sprintf(msg,"%d",READY);
	write(pipe_chld_prnt[1],msg,strlen(msg));
	     
	i=0;
	int command=0;
	int pivot; 
	srand(time(NULL));

	// enter in while loop broken by parent kill signal
    while(1)
    {
    		
    	// read command 	
		char *sig=(char*)malloc(10*sizeof(char));
		while(read(pipe_prnt_chld[0], sig, sizeof(sig)) <=0);
 		command=atoi(sig);
 		free(sig);
        
        switch(command) 
        {
        	case REQUEST:
        		{		
        	     	int a=0;
        			//printf("--- Child %d receives REQUEST\n",ID);
					if(size==0) 
					{
						char *buffer=(char*)malloc(10*sizeof(char));
    	 				sprintf(buffer,"%d",-1);
   			  			write(pipe_chld_prnt[1],buffer,strlen(buffer));
   			  			printf("--- Child %d receives REQUEST and replies -1\n",ID);
   			  			free(buffer);
					}
					else
					{
						char *buffer=(char*)malloc(10*sizeof(char));
    	 				int index=rand()%size;
    	 				sprintf(buffer,"%d",array[index]);
   			  			write(pipe_chld_prnt[1],buffer,strlen(buffer));
   			  			printf("--- Child %d receives REQUEST and replies %d\n",ID,array[index]);
   			  			free(buffer);
					}
        			break;
        		}	
        	case PIVOT:
        		{
        			//printf("--- Child %d receives PIVOT\n",ID);
        			char *buffer=(char*)malloc(10*sizeof(char));
        			while(read(pipe_prnt_chld[0], buffer, sizeof(buffer)) <=0);
        			pivot=atoi(buffer);
        			//printf("--- Child %d receives PIVOT value %d\n",ID,pivot);
        			free(buffer);	

        			int no_greater=0;
        			for(i=0;i<size;i++) if(array[i]>pivot) no_greater++;

        			buffer=(char*)malloc(10*sizeof(char));
	 				sprintf(buffer,"%d",no_greater);
			  		write(pipe_chld_prnt[1],buffer,strlen(buffer));
			  		printf("--- Child %d receives PIVOT value %d and replies %d\n",ID,pivot,no_greater);
			  		free(buffer);		
        			break;
        		}
        	case SMALL:
        		{

        			//printf("--- Child %d receives SMALL\n",ID);
        			//delete all small elements
        			int k=0;
        			for(i=0;i<size;i++)
        				if(array[i]>=pivot)
        				{
        					array[k++]=array[i];
        				}
        			size=k;	
        			printf("--- Child %d receives SMALL\n",ID);
        			//printf("--- Child %d done SMALL size =%d\n",ID,size);
        			break;
        		}

        	case LARGE:
        		{
        			//printf("--- Child %d receives LARGE\n",ID);
        			int k=0;
        			for(i=0;i<size;i++)
        				if(array[i]<=pivot)
        				{
        					array[k++]=array[i];
        				}
        			size=k;
        			//printf("--- Child %d done LARGE size =%d\n",ID,size);
        			printf("--- Child %d receives LARGE\n",ID);
        			break;						
        		}
        }
    } 

	close(pipe_prnt_chld[0]); 

	return 0;
}
