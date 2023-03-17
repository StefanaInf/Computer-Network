#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define fifo_name "FIFO"
#define fifo_write "fifo2"

int main()
{
	char message[50];
	int num, fd, fw;
	
	fd = open(fifo_name, O_WRONLY); //write only
	printf("Input command: \n");
	
	while(fgets(message,50,stdin), !feof(stdin)){
		if((num = write(fd, message, strlen(message))) == -1){
			perror("[client]Write error");
		}
	}
		
}
