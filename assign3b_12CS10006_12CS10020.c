/********************** Operating Systems Laboratory *************************/
/********************* Assignment 3 B (Rock Paper Scissor) ************************/

//header files inclusion

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

//#define ready 0
#define paper 1
#define scissor 2
#define rock 3

// ready for childs
int ready1=0;
int ready2=0;

// scores of palyers
double player1=0;
double player2=0;

// increment point of palyer 
void inc_point(int a,int b)
{
	if(a==rock && b==scissor) player1+=1;
	if(a==rock && b==paper) player2+=1;
	if(a==scissor && b==paper) player1+=1;
	if(a==b) {	player1+=0.5; player2+=0.5; }
  return;	
}

// signal handler for child1
void usr1handp1(){ready1=1;}
void usr2handp1(){printf("player1 Lost the game\n"); exit(0);}
void inthandp1(){printf("player1 Won the game\n"); exit(0);}

// signal hander for child 2
void usr1handp2(){ready2=1;}
void usr2handp2(){printf("player2 Lost the game\n");exit(0);}
void inthandp2(){printf("player2 Won the game\n");exit(0);}

// main programm
int main()
{
	int pipe1[2];
	int pipe2[2];
    
	pid_t pid1;
	pid_t pid2;

	if(pipe(pipe1)<0) perror("pipe1:");
	if(pipe(pipe2)<0) perror("pipe2:");

	pid1=fork();
	if(pid1<0) perror("fork1");
	if(pid1==0){ // child 1
		signal(SIGUSR1,usr1handp1);
		signal(SIGUSR2,usr2handp1);
		signal(SIGINT,inthandp1);

		while(1){	
		// wait for ready signal
		while(!ready1);
			// generate random nuumber
			srand(time(NULL));
			int rnd=1+rand()%3;
			char p1[20]; 
			sprintf(p1,"Random no generated in child1: %d\n",rnd);
			write(STDOUT_FILENO,p1,strlen(p1));
			// close read end
			close(pipe1[0]);
			char *buffer=(char*)malloc(10*sizeof(char));
			sprintf(buffer,"%d",rnd);

			write(pipe1[1],buffer,strlen(buffer));	
			free(buffer);
			ready1=0;
		}

	} 
	else  // parent
	{	
		
			pid2=fork();
			if(pid2<0) perror("fork1");
			if(pid2==0){ // child 2
				signal(SIGUSR1,usr1handp2);
				signal(SIGUSR2,usr2handp2);
				signal(SIGINT,inthandp2);

			while(1){	
			// wait for ready signal
				while(!ready2);
			// generate random nuumber
		    //srand(time(NULL));
			int rnd=1+rand()%3;
			char p2[20]; 
			sprintf(p2,"Random No generated in child2: %d\n" , rnd);
			
	        //printf("Random No generate in child2 is %d" , rnd); 

	        write(STDOUT_FILENO,p2,strlen(p2));
			// close read end
			close(pipe2[0]);
			char *buffer=(char*)malloc(10*sizeof(char));
			sprintf(buffer,"%d",rnd);

			write(pipe2[1],buffer,strlen(buffer));	
			free(buffer);
			ready2=0;
		}
		}
		else
		{
			while(1)
			{
				// send ready to child 1 and child 2
				sleep(1);
				kill(pid1,SIGUSR1);
				kill(pid2,SIGUSR1);

				// read from pipe1
				// will be used to communicate through pipe
				char *buffer=(char*)malloc(10*sizeof(char));
				//close write side. don't need it.
				close(pipe1[1]);
				// read ID
				while(read(pipe1[0],buffer, sizeof(buffer)) <=0);	

				int child1=atoi(buffer);

				free(buffer);

				// read from pipe2
				// will be used to communicate through pipe
				buffer=(char*)malloc(10*sizeof(char));
				//close write side. don't need it.
				close(pipe2[1]);
				// read ID
				while(read(pipe2[0],buffer, sizeof(buffer)) <=0);	

				int child2=atoi(buffer);

				free(buffer);

				inc_point(child1,child2);
				//printf("child 1 send =%d child 2 send =%d\n",child1,child2);
				printf ("score of player1 :%lf \n",player1);
                 printf("score of player2 :%lf \n",player2);
                 printf("\n");
				if(player1>10 || player2>10) break;		
			}

				int win=2;
				if(player1>10 && player2<=10) // player 1 wins
					win=1;

				if(player2>10 && player1<=10) ;// player 2  wins	 
			     
				if(player1>10 && player2>10)  // tie
				{
					srand(time(NULL));
					int randn1=rand();
					int randn2=rand();
					if(randn1>randn2) // child 1 wins
						win=1;
						//else ;// child2 wins 					
				} 
				if(win==1){
					kill(pid1,SIGINT);
					kill(pid2,SIGUSR2);
				}
				else{
					kill(pid2,SIGINT);
					kill(pid1,SIGUSR2);
			}

			

		}
	}

return 0;	
}