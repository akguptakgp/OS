#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	char *buffer=(char*)malloc(100*sizeof(char));
	sprintf(buffer,"just a program to test priting to stdout");
	write(STDOUT_FILENO,buffer,strlen(buffer));
	free(buffer);
return 0;
}
