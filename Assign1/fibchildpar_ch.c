/****************************** 		Operating Systems Laboratory  		**********************/ 
/****************************** 		Assignment 1 (fibchildpar_ch.c)  			**********************/
/****************************** 		Ankit Kumar Gupta (12CS100006)  	**********************/
/****************************** 		Gaurav Kumar  (12CS10020)  			**********************/

// header file inclusion
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


// function to create nth fibonacci number
int fib(int n)
{
	if(n==1 || n==2) return 1;
	else
		return fib(n-1)+fib(n-2);	
}

// main programm 
int main()
{
	int n; 		// number of fibonacci numbers to print
	int status;	// used to store exit status return by wait() function  


	// scan n
	printf("enter n : ");
	scanf("%d",&n);

	int i;
	
	for(i=1;i<=n;i++)
	{
		pid_t pid=fork();
		if(pid<0) printf("Process creation unsuccessful\n");
			if(pid==0) { // child process 
							//computer current fibonacci number and print it	
			int tmp=fib(i);  
			printf("in process with pid %d  %dth fibonacci number is %d\n",getpid(),i,tmp);
			exit(tmp); 
		}		
	}

	for(i=0;i<n;i++)
	{
		wait(&status); // wait for all child processes to exit	
	}


	return 0;			// return from main process
}