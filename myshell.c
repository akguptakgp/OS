/********************** Operating Systems Laboratory *************************/
/********************* Assignment 2 A (shell design) ************************/

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

case pwd:
	cwd=getcwd(buffer,PATH_MAX+1);
	if(cwd!=NULL)
		printf("%s\n",cwd);
 	else
 		perror("pwd : Error");	
 	break;

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
case ext:	
 	exit(0);
	break;
case call_:	
	
	pid=fork();
	if(pid==0){
	i=1;
	while(argv[i]!='\0' && strcmp(argv[i],"&")) i++;
	if(argv[i]!='\0' && !strcmp(argv[i],"&")) argv[i]='\0';  
	
	//int i=0;
	//while(argv[i]!='\0') printf("%s\n",argv[i++]);

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
	break;
 	}
}
return 0;
}	
