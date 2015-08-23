#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#define msgsize 200
#define maxclinet 100

typedef struct msgbuf_{
    long mtype;       /* message type, must be > 0 */
    char mtext[msgsize];    /* message data */
}msgbuf;

typedef char client_list[25];
int up_msgid;
int down_msgid;
key_t up_key;
key_t down_key;
client_list clst[25];
int clst_indx=0;
msgbuf message;

int up_msgid;
int down_msgid;
key_t up_key;
key_t down_key;

int startswith(char *s,char *p)
{
	int rslt=1;
	int i=0;
	while(s[i]!='\0' && p[i]!='\0')
	{
		if(s[i]!=p[i]) rslt=0;
		i++;
	}

	return rslt;	
}

void update_list()
{
	int size=msgrcv(down_msgid,&message,sizeof(message.mtext),getpid()%maxclinet+1,0);
	if(size<0) perror("server msgrcv");
	
	if(startswith(message.mtext,"LIST"))
	{	
		//printf("list of all clients currently online \n");
		//printf("%s\n",message.mtext);
		char *p=&message.mtext[5];
		int i=0;
		clst_indx=0;
		while(p[i]!='\0')
		{
			int j=0;
			while(p[i]!='\0' && p[i]!='>')
			{
				clst[clst_indx][j++]=p[i++];
			}
			clst[clst_indx++][j]='\0';
			i++;
			if(p[i]=='\0') break;
			i++;
		}
		// for(i=0;i<clst_indx;i++)
		// 	printf("%s\t",clst[i]);
		// printf("\n");
	}
	else
		printf("no it is not list of clients\n");
}

void exit_duplicate()
{
	printf("server killed me because i was duplicate client\n");
	exit(0);
}
int main()
{
	signal(SIGUSR1,update_list);
	signal(SIGUSR2,exit_duplicate);
	//printf("%d\n",getpid()%maxclinet);
	
	// key generation to avoide conflicts
	char *path = "/usr";
	up_key = ftok(path,'P');
	down_key=ftok(path, 'Q');

	//printf("%d %d\n",up_key,down_key);

		// create message queues 
	if((up_msgid=msgget(up_key,IPC_CREAT|0666))<0) perror("message queue create:");
	if((down_msgid=msgget(down_key,IPC_CREAT|0666))<0) perror("message queue create:");
	//printf("up_queue=%d down_queue=%d\n",up_msgid,down_msgid);

	

	// ask client for chat ID
	//printf("client pid=%d\n",getpid());
	printf("please enter your Chat ID\n");
	
	char chat_id[25];
	scanf("%s",chat_id);
	
	
	message.mtype=getpid()%maxclinet+1;
	sprintf(message.mtext,"NEW %s",chat_id);

	int s=msgsnd(up_msgid,&message,sizeof(message.mtext),0);
	if(s==-1) perror("client msgsnd");
	strcpy(clst[clst_indx++],chat_id);

	// get list of other clients
	int size=msgrcv(down_msgid,&message,sizeof(message.mtext),getpid()%maxclinet+1,0);
	if(size<0) perror("server msgrcv");
	
	if(startswith(message.mtext,"LIST"))
	{	
		printf("list of all clients currently online \n");
		//printf("%s\n",message.mtext);
		char *p=&message.mtext[5];
		int i=0;
		clst_indx=0;
		while(p[i]!='\0')
		{
			int j=0;
			while(p[i]!='\0' && p[i]!='>')
			{
				clst[clst_indx][j++]=p[i++];
			}
			clst[clst_indx++][j]='\0';
			i++;
			if(p[i]=='\0') break;
			i++;
		}
		for(i=0;i<clst_indx;i++)
			printf("%s\t",clst[i]);
		printf("\n");
	}
	else
		printf("no it is not list of clients\n");

	// enter in loop and ask 
	while(1)
	{	
		printf("do u want to send a message y/n\n");
		char response='\n';

		//while(response!='y' || response!='n'){
			//printf("renter\n");	
			scanf("\n%c",&response);
		  //}
		
		if(response=='y')
		{	
			int i=0;
			char user_to_send[25];
			while(1)
			{
			printf("please choose a client from the following online clients to send message\n");
			for(i=0;i<clst_indx;i++)
				printf("%s\t",clst[i]);
				
				printf("\n");
				scanf("%s",user_to_send);
				int flag=0;
				for(i=0;i<clst_indx;i++)
					if(!strcmp(clst[i],user_to_send)) flag=1;
				if(!flag) {
					printf("oops!!! there was error client not exist please select again\n");
				}
				else
					break;

			}
			
			getchar(); // consume null character
			printf("please type the message you want to send to %s\n",user_to_send);
			char msg_string[msgsize];
			gets(msg_string);
			//printf("read done\n");
			sprintf(message.mtext,"MSG<%s><%s>",msg_string,user_to_send);
			//printf("%s\n",message.mtext);
			message.mtype=getpid()%maxclinet+1;

			// send message to server
			int s=msgsnd(up_msgid,&message,sizeof(message.mtext),0);
			if(s==-1) perror("client msgsnd");
		}	
		// message receive
		int size=msgrcv(down_msgid,&message,sizeof(message.mtext),getpid()%maxclinet+1,IPC_NOWAIT);
		//if(size<0) perror("client message receive\n");
		if(size>0) 
		{	
			if(startswith(message.mtext,"MSG"))
			{	
				//printf("server send message\n\n\n\n");
				char *p=&message.mtext[4];
				int i=0;
				int parse_indx=0;
				char parse[3][100];

				while(p[i]!='\0')
				{
					int j=0;
					while(p[i]!='\0' && p[i]!='>')
					{
						parse[parse_indx][j++]=p[i++];
					}
					parse[parse_indx++][j]='\0';
					i++;
					if(p[i]=='\0') break;
					i++;
				}
				printf("message received=\"%s\"\n",parse[0]);
				printf("time=%s\n",parse[1]);
				printf("sender chat ID =\"%s\"\n",parse[2]);
			}
			if(startswith(message.mtext,"LIST"))
			{	
				//printf("list of all clients\n");
				char *p=&message.mtext[5];
				int i=0;
				clst_indx=0;
			
				while(p[i]!='\0')
				{
					int j=0;
					while(p[i]!='\0' && p[i]!='>')
					{
						clst[clst_indx][j++]=p[i++];
					}
					clst[clst_indx++][j]='\0';
					i++;
					if(p[i]=='\0') break;
					i++;
				}
			}
		}
		
		//break;
	}

return;	
}