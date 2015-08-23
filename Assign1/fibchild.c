/****************************** 		Operating Systems Laboratory  		**********************/ 
/****************************** 		Assignment 1 (fibchild.c)  			**********************/
/****************************** 		Ankit Kumar Gupta (12CS100006)  	**********************/
/****************************** 		Gaurav Kumar  (12CS10020)  			**********************/

// header file inclusion
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// main programm 
int main()
{
	int n; 		// number of fibonacci numbers to print
	int i=1;	// counter of the current fibonacci number to be printed
	int status;	// used to store exit status return by wait() function  

	printf("enter n : ");
	scanf("%d",&n);

	int curr=1,prev=1; // current and previous fibonacci number initilized with 1

	while(i<=n)       // loop to compute n fibonacci number
	{	

		pid_t pid=fork(); // create a new process
		if(pid<0)
		{
			printf("Process creation unsuccessful Exiting\n");
			exit(0);
		}
		if(pid==0)		// child process
		{
			// since child process get the same address space as parent process with all variables initilized with same value as parent
			// so we can use i and curr and prev because each iteration we create a new process so it gets updated values
			printf("in process with pid=%d, parent process pid=%d\n",getpid(),getppid());
			if(i==1 || i==2) 
					curr=1; 		// if(n==1 || n==2) fib(n)=1;	
			else
					curr+=prev; 	// else fib(n)=fib(n-1)+fib(n-2)

			printf("%dth fibonacci number is: %d\n",i,curr);
			exit(curr);			// exit with current fibonacci numer as status 
			
		}	
		else		// parent process
		{
			pid_t pid=wait(&status);	// wait for child process to exit
			printf("process with pid %d exited\n",pid);
			int tmp=WEXITSTATUS(status);	// get current fibonacci number from status variable
			prev=curr;						// update prev and curr prev fibonacci number
			curr=tmp;
			i++;							//increment i
		}

	}
	return 0;			// return from main process
}