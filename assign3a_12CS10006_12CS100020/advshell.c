/********************** Operating Systems Laboratory *************************/
/********************* Assignment 3 A (shell design with piping) ************************/

//header files inclusion
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>

// enum declaration to parse command
enum command {
cd,
pwd,
mkdir_,
rmdir_,
ls_,
cp,
ext,
call_
};

// split the command into array of strings
void  split(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' &&
                 *line != '\t' && *line != '\n')
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
}


// parse the command string
enum command parse(char *buffer)
{
if(strcmp(buffer,"cd")==0)
	return cd;
else if(strcmp(buffer,"pwd")==0)
	return pwd;
else if(strcmp(buffer,"mkdir")==0)
	return mkdir_;
else if(strcmp(buffer,"rmdir")==0)
	return rmdir_;
else if(strcmp(buffer,"ls")==0)
	return ls_;
else if(strcmp(buffer,"cp")==0)
	return cp;
else if(strcmp(buffer,"exit")==0)
	return ext;
else
	return call_;
}


int is_hidden(const char *name)
{
  return (name[0] == '.');
}

int main()
{

char  line[1024];             /* the input line                 */
char  *argv[64];              /* the command line argument  to be passed to execvp   */

 while(1)
 {
 	char *buffer =(char*)malloc((PATH_MAX+1)*sizeof(char));	// to store current directory
 	char *cwd=getcwd(buffer,PATH_MAX+1); // current working directory
 	if(buffer!=NULL)
	{
		printf("%s:~>",cwd); // print current working directory
	}
	else
	{
 		perror("error");
 		exit(0);
 	}

 gets(line);  // read command
 split(line, argv); // split command


// variables to be used inside switch
int success;
DIR* dirp;
struct dirent* direntp;
struct stat buf;
int i;
pid_t pid;
switch(parse(argv[0]))
{

// change directory
case cd: // cd takes only first argument and ignores rest
	// default is home directory
	if(argv[1]!='\0')
		success=chdir(argv[1]);
	else
	{
		char * ptr_path;
		ptr_path = getenv ("HOME");
		//printf("%s\n",ptr_path);
		success=chdir(ptr_path);
	}
	if(success==-1){
	 	sprintf(buffer,"cd : can not change directory to '%s'",argv[1]);
	 	perror(buffer);
	 }
        break;

// print current directory
case pwd:
	cwd=getcwd(buffer,PATH_MAX+1);
	if(cwd!=NULL)
		printf("%s\n",cwd);
 	else
 		perror("pwd : Error");
 	break;

// make directory
case mkdir_: // takes a list of arguments as input and creates directory for all of these inputs
	i=1;
	while(argv[i]!='\0'){
		success=mkdir(argv[i],S_IRWXU);  //read, write, execute/search by others
		if(success==-1){
			sprintf(buffer,"mkdir: cannot create directory '%s'",argv[i]);
			perror(buffer);
		}
	i++;
	}

	if(argv[1]=='\0') printf("mkdir: missing operand\n");

    break;

// remove directory
case rmdir_:

	if(argv[1]=='\0') printf("rmdir: missing operand\n");
	i=1;
	while(argv[i]!='\0')
	{
		success=rmdir(argv[i]);
		if(success==-1){
			sprintf(buffer,"rmdir: failed to remove '%s'",argv[i]);
			perror(buffer);
		}
		i++;
	}
	break;

// list files inj current directory
case ls_:

	if((argv[1]!='\0' &&  strcmp(argv[1],"-l"))) // (argv[1]!='\0') &&
	{
		printf("Error: ls only -l supported\n"); continue;
	}
    cwd=getcwd(buffer,PATH_MAX+1);
    if(buffer==NULL)
    {
		perror("error");
		continue;
 	}
 	dirp = opendir(cwd);
 	if(dirp==NULL)
 	{
		perror("error");
		continue;
 	}
	else{
    	for(;;)
    	 {
	        direntp = readdir(dirp);
	        if( direntp == NULL ) break;
	        else
	   			{

	   				    if(!is_hidden(direntp->d_name))
	   						if(argv[1]!='\0' && !strcmp(argv[1],"-l"))
	   						{

	   							if(stat(direntp->d_name,&buf)!=-1)
	   							{
	   								struct passwd *pw = getpwuid(buf.st_uid);
									struct group  *gr = getgrgid(buf.st_gid);
									if(pw!=NULL && gr!=NULL)
									{
	   								printf( (S_ISDIR(buf.st_mode)) ? "d" : "-");
								    printf( (buf.st_mode & S_IRUSR) ? "r" : "-");
								    printf( (buf.st_mode & S_IWUSR) ? "w" : "-");
								    printf( (buf.st_mode & S_IXUSR) ? "x" : "-");
								    printf( (buf.st_mode & S_IRGRP) ? "r" : "-");
								    printf( (buf.st_mode & S_IWGRP) ? "w" : "-");
								    printf( (buf.st_mode & S_IXGRP) ? "x" : "-");
								    printf( (buf.st_mode & S_IROTH) ? "r" : "-");
								    printf( (buf.st_mode & S_IWOTH) ? "w" : "-");
								    printf( (buf.st_mode & S_IXOTH) ? "x" : "-");
								    printf("  %d",buf.st_nlink);
								  	struct tm * timeinfo;
								  	timeinfo = localtime (&buf.st_mtime);
								  	strftime (buffer,80,"%h %d %H:%M",timeinfo);
								    printf( " %s %s%9d %s  %s\n",pw->pw_name,gr->gr_name,buf.st_size,buffer,direntp->d_name );
								    }
								    else
								    {
								    	perror("error");
								    }

	   							}
	   							else
	   								perror("error");
	   						}
	   						else
		   						printf( "%s\n", direntp->d_name );

	   			}
	    }}
   closedir( dirp );
   break;

// copy file1 into file2
case  cp:
	if(argv[1]=='\0'){
		printf("cp: missing file operand"); continue;
	}
	else if(argv[2]=='\0'){
		printf("cp: missing destination file operand after `%s'\n",argv[1]);
		continue;
	}

	if(access(argv[1],F_OK)!= -1)
	{
		if(access(argv[1],R_OK) != -1)
		{
			if(access(argv[2],F_OK) != -1) // file 2  exist
			{
		        if(access(argv[2],W_OK) != -1) // file 2  write permission OK
				{

					struct stat file1;
					struct stat file2;
					if((stat(argv[1],&file1)!=-1)&&(stat(argv[2],&file2)!=-1))
					{
						if(file1.st_mtime>file2.st_mtime) // file1 last modification time more recent than file2
						{
							FILE *file1,*file2;
							file1=fopen(argv[1],"r");
							file2=fopen(argv[2],"w");
							char ch;
							 while( ( ch = fgetc(file1) ) != EOF )
     								 fputc(ch, file2);
     						fclose(file1);
     						fclose(file2);
						}
						else // file2 last modification time more recent than file1
						{
							printf("Error: %s is more recent than %s\n",argv[2],argv[1]);
						}

					}
					else // file stat failed
					{
						if(stat(argv[1],&file1)==-1) // file stat failed due to file 1
						{
							sprintf(buffer,"Error %s",argv[1]);
		  					perror(buffer);
						}
						if(stat(argv[2],&file2)==-1)    // file stat failed due to file 2
						{
							sprintf(buffer,"Error %s",argv[2]);
		  					perror(buffer);
						}

					}

				}
				else 	// file 2  write permission not OK
				{
					sprintf(buffer,"Error %s",argv[2]);
		  			perror(buffer);
				}
			}
			else // file 2 not exist
			{
				FILE *file1,*file2;
				file1=fopen(argv[1],"r");
				file2=fopen(argv[2],"w");
				char ch;
				while( ( ch = fgetc(file1) ) != EOF )
     				 fputc(ch, file2);
     			fclose(file1);
     			fclose(file2);

			}

		}
		else {	// file 1 read permission
			sprintf(buffer,"Error %s",argv[1]);
		  	perror(buffer);
		  }
    }
	else if(errno==ENOENT) // file 1 not exist
	 {

	 	sprintf(buffer,"Error %s",argv[1]);
		perror(buffer);
	 }

	break;

// exit
case ext:
 	exit(0);
	break;

// execute a function
case call_:

	i=0;
	while(argv[i]!='\0')
	{
		if(!strcmp(argv[i],"|")) break;
		else i++;
	}

	if(argv[i]=='\0') // simple call
	{
		pid=fork();
		if(pid==0){
		i=1;
		while(argv[i]!='\0' && strcmp(argv[i],"&")) i++;
		if(argv[i]!='\0' && !strcmp(argv[i],"&")) argv[i]='\0';

		int no_args=0;
		while(argv[no_args]!='\0') no_args++;// printf("%s\n",argv[no_args++]);
		//printf("%d\n",no_args);

		if(no_args>1){
		if(!strcmp(argv[no_args-2],">"))// file redirection
		{
			close(STDOUT_FILENO);

			//printf("st_mode\n");
			int file ;//= open(argv[no_args-1], O_CREAT|O_APPEND | O_WRONLY);
	    	FILE *f=fopen(argv[no_args-1],"w");
	    	file=fileno(f);
	    	if(file < 0)    perror("File open Error:");

		    //Now we redirect standard output to the file using dup2
	    	if(dup2(file,STDOUT_FILENO) < 0)  perror("Dup error:");
		}

		if(!strcmp(argv[no_args-2],"<"))// file redirection
		{
			close(STDIN_FILENO);
			//printf("st_mode\n");
			int file ;//= open(argv[no_args-1], O_CREAT|O_APPEND | O_WRONLY);
	    	FILE *f=fopen(argv[no_args-1],"r");
	    	file=fileno(f);
	    	if(file < 0)    perror("File open Error:");

		    //Now we redirect standard output to the file using dup2
	    	if(dup2(file,STDIN_FILENO) < 0)  perror("Dup error:");
		}

		}


	 	int s=execvp(argv[0],argv);


		if(s==-1) {printf("failed\n");exit(0);}
		}
		else{
			int st;
			i=1;
			while(argv[i]!='\0' && strcmp(argv[i],"&")) i++;
			if(argv[i]=='\0') {
		   		pid_t pid=wait(&st);
			}
		    continue;
		}
	}
	else  // redirect output of 1st prgramm to other and so on
	{
		i=0;
		int no_com=1;
		while(argv[i]!='\0')
		{
			if(!strcmp(argv[i++],"|")) no_com++;
		}

		//printf("total no. of command to be executeed %d\n",no_com);

		int start;
		int p=0;

		char *last=NULL;
		char *current=NULL;
		
		char *buffer=NULL;
		for(i=0;i<no_com;i++)
		{

			// get command to be executed
			start=p;
			while(argv[p]!='\0' && strcmp(argv[p],"|") ) p++;
			argv[p]='\0'; p++;

			char *command[10];	
			int pcom=0;
			while(argv[start]!='\0') command[pcom++]=strdup(argv[start++]);
			command[pcom]='\0';

			current=strdup(command[0]);

			int inpipe[2];
			int outpipe[2];	

		// create a new pipe
			if(pipe(inpipe) <0) printf("pipe1:");
			if(pipe(outpipe) <0) printf("pipe2:");
			//printf("pipes are %d %d %d %d\n",inpipe[0],inpipe[1],outpipe[0],outpipe[1]);


			pid_t pid=fork();
			if(pid<0) printf("fork:");
			if(pid==0) // child will read from inpipe and will write to ouput
			{
				if(close(inpipe[1])<0) perror("inpipe[1] child close");
				if(close(outpipe[0])<0) perror("outpipe[0] child close");

				if(i!=no_com-1)  // not last command
				{
	    			if(dup2(outpipe[1],STDOUT_FILENO) < 0) perror("Dup1 error:\n\n\n\n\n");
	   			}
				if(i!=0) // not first command
				{
	    			if(dup2(inpipe[0],STDIN_FILENO) < 0)  perror("Dup2 error:\n\n\n\n\n");
				}

				int s=execvp(command[0],command);
				if(s==-1) {printf("failed\n");exit(0);}
				exit(0);

			}
			else
			{

				if(buffer!=NULL){
					if(close(inpipe[0])<0) perror("inpipe[0] parent close:");
					if(write(inpipe[1],buffer,strlen(buffer))<0) perror("write parent:");
					close(inpipe[1]);
					char display[1000];
					sprintf(display,"the ouput \"%s\"  from  \'%s\'  was fed into  \'%s\', %d \n",buffer,last,current,strlen(buffer));
					write(STDOUT_FILENO,display,strlen(display));
				}

				//printf("executing parent %d %d %d\n",i,getpid(),getppid());
				int status;
				waitpid(pid,&status,0);

			// parent does not need read end of inpipe 
			// parent does not need write end of outpipe
			
			if(close(outpipe[1])<0) perror("outpipe[1] parent close:");
			 	
			 if(i!=no_com-1){          // not last command

				buffer=(char*)malloc(1000*sizeof(char));
			
				while(read(outpipe[0],buffer,1000)<0);
			
				//printf("parent read done %s %d\n",buffer,strlen(buffer));

				}
				last=command[0];
			}
		}

	}

	break;
 	}
}

return 0;
}
