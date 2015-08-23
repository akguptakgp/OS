#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h> 
#include <signal.h>

#define maxclinet 100
#define msgsize 200

typedef struct {
    long mtype;       /* message type, must be > 0 */
    char mtext[msgsize];    /* message data */
}msgbuf;

typedef struct node
{
	pid_t cpid;
	char cchatid[25];
}client_list;

int up_msgid;
int down_msgid;

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

void exithandler()
{	
	if(msgctl(up_msgid,IPC_RMID,NULL)<0) perror("message queue remove");
	if(msgctl(down_msgid,IPC_RMID,NULL)<0) perror("message queue remove");
	exit(0);	
}
int main()
{
	signal(SIGINT,exithandler);
	key_t up_key;
	key_t down_key;

	// key generation to avoide conflicts
	char *path = "/usr";
	up_key = ftok(path,'P');
	down_key=ftok(path, 'Q');

	//printf("%d %d\n",up_key,down_key);

	// create message queues 
	if((up_msgid=msgget(up_key,IPC_CREAT|0666))<0) perror("message queue create:");
	if((down_msgid=msgget(down_key,IPC_CREAT|0666))<0) perror("message queue create:");
	//printf("up_queue=%d down_queue=%d\n",up_msgid,down_msgid);

	//message=(msgbuf*)malloc(sizeof(msgbuf));
	client_list clst[10];
	int clst_indx=0;
	msgbuf message;
	int i;
	while(1)
	{

		// read any message from up_message queue	
	 	int size=msgrcv(up_msgid,&message,sizeof(message.mtext),0,0);
	 	if(size<0) perror("server msgrcv");
	 	if(startswith(message.mtext,"NEW")) 
	 	{
		 	struct msqid_ds buf;
		 	int s=msgctl(up_msgid,IPC_STAT,&buf);
		 	if(s!=0) perror("msgctl");
		 	//printf("pid of sender %d\n",buf.msg_lspid);
	 		
	 		// check if client exists
	 		int is_exist=0;
	 		char newid[25];
	 		strcpy(newid,&message.mtext[4]);
	 		for(i=0;i<clst_indx;i++)
	 			if(!strcmp(clst[i].cchatid,newid)) is_exist=1;

	 		if(!is_exist)
	 		{
		 		clst[clst_indx].cpid=buf.msg_lspid;
		 		strcpy(clst[clst_indx++].cchatid,&message.mtext[4]);
		 		printf("new client added\n");
				printf("ChatID=%s PID=%d total clients=%d\n",clst[clst_indx-1].cchatid,clst[clst_indx-1].cpid,clst_indx);		
		 			
		 		// prepare list of all clients
				 strcpy(message.mtext,"LIST");
				 for(i=0;i<clst_indx;i++)
				 {
				 	strcat(message.mtext,"<");
				 	strcat(message.mtext,clst[i].cchatid);
				 	strcat(message.mtext,">");
				 }
				//printf("LIST of clients %s\n",message.mtext);
		 		// send list to all clients
				for(i=0;i<clst_indx;i++)
		 		{
		 			message.mtype=(clst[i].cpid)%maxclinet+1;
				 	int s=msgsnd(down_msgid,&message,sizeof(message.mtext),0);
				 	//printf("message send to %s\n",clst[i].cchatid);
				 	if(s<0) perror("server msgsnd");
				 	// send signal to update list
				 	if(i!=clst_indx-1) kill(clst[i].cpid,SIGUSR1);
				}
			}
			else
			{
				printf("client exists already\n");
				printf("new client requested denied\n");
				kill(buf.msg_lspid,SIGUSR2);
			}		
	 	}
	 	if(startswith(message.mtext,"MSG")) 
	 	{
		 	//printf("server=%s\n",message.mtext);
			struct msqid_ds buf;
			int s=msgctl(up_msgid,IPC_STAT,&buf);
			if(s!=0) perror("msgctl");
			//printf("pid of sender %d\n",buf.msg_lspid);
		 	
		 	//determine chat id using map list
		 	char sender_chatid[25];
			//printf("no. of sender=%d\n",clst_indx);			 			
	 		for(i=0;i<clst_indx;i++)
	 		{
	 			if(clst[i].cpid==buf.msg_lspid) // sender found
	 			{
	 				strcpy(sender_chatid,clst[i].cchatid);
	 			}
	 		}
	 		// parse message
 			char *p=&message.mtext[4];
	 		int parse_indx=0;
			char parse[2][100];
			i=0;
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
	 			
	 		pid_t receipt_pid;
	 		for(i=0;i<clst_indx;i++)
	 		{
	 			if(!strcmp(parse[1],clst[i].cchatid)) // receipt found
	 				receipt_pid=clst[i].cpid;
	 		}

	 		printf("server received '%s' from '%s'\n",message.mtext,sender_chatid);

	 		// prepare message
			message.mtype=receipt_pid%maxclinet+1;
	 		struct tm * timeinfo;
	 		timeinfo = localtime (&(buf.msg_stime));
	 		char buffer[80];
	 		strftime (buffer,80,"%h %d %H:%M",timeinfo);
	 		sprintf(message.mtext,"MSG<%s><%s><%s>",parse[0],buffer,sender_chatid);
			//printf("%s\n",message.mtext);
	 		s=msgsnd(down_msgid,&message,sizeof(message.mtext),IPC_NOWAIT);
			if(s<0) perror("server msgsnd");
			printf("server sent '%s' to '%s'\n",message.mtext,parse[1],clst[i].cchatid);
		}
		
	}
		 // 	int s;
			// waitpid(pid,&s);
			// printf("wait done for pid=%d\n",pid);
			// printf("parent down\n");
// delete the message

return;	
}