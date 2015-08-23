/****************************** 		Operating Systems Laboratory  		**********************/ 
/****************************** 		Assignment 1 (fibchildpar.c)  			**********************/
/****************************** 		Ankit Kumar Gupta (12CS100006)  	**********************/
/****************************** 		Gaurav Kumar  (12CS10020)  			**********************/

// header file inclusion
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


// declare pid value pair
// we will use this to store value returned by process with particular PID 
 typedef struct node
 {
 	pid_t pid;
 	int value;
 }pid_value_pair;

// function to compute nth fibonacci number
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
    pid_value_pair *fibarr; // pid_value pair array 
	
    // scan n
	printf("enter n : ");
	scanf("%d",&n);

	// allocate memory to array
	fibarr=(pid_value_pair*)malloc(n*sizeof(pid_value_pair));
	
	int i;
	// run a loop and create n parallel process where ith process computes ith fibonacci number	
	for(i=1;i<=n;i++)
	{
		pid_t pid=fork();
		if(pid<0) printf("Process creation unsuccessful\n");
		if(pid>0) fibarr[i-1].pid=pid;
		if(pid==0) {	
			exit(fib(i));  // exit with the current fibonacci number (using this method we can only store upto 8 digits in status register
						   // for higher n we need to use pipes (shared memeory))
		}		
	}

	for(i=0;i<n;i++) // wait for child processed to finish
	{

		waitpid(fibarr[i].pid,&status,0);	
		fibarr[i].value=WEXITSTATUS(status); 	// store status information in pid value array
	}

	// print all fibonacci number
	for(i=0;i<n;i++)
	printf("from process with pid=%d %dth fibonacci number %d\n",fibarr[i].pid,i+1,fibarr[i].value);

	return 0;			// return from main process
}