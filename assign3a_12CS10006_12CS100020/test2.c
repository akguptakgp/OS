#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{

char *buffer=(char*)malloc(100*sizeof(char));
sprintf(buffer,"just a program to test scaning from stdin\n");
write(STDOUT_FILENO,buffer,strlen(buffer));
free(buffer);

char string[1000];
gets(string);

buffer=(char*)malloc(1000*sizeof(char));
sprintf(buffer,"scan done from stdin \"%s\" \n",string);
write(STDOUT_FILENO,buffer,strlen(buffer));
free(buffer);
return 0;
}
