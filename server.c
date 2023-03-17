 #include <stdio.h>
 #include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#define fifo_name "FIFO"
#define fifo_write "fifo2"

int main()
{
	char message[50];
	int num, fd, fw, k = 0;
	int sockpair1[2]; 
	pid_t pid;
	int user_logged_in = 0; //it becomes 1 of a user is logged in
	char username[50];
	
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair1) < 0) { 
        	perror("Err... socketpair"); 
        	exit(1); 
       }
	
	//create a fifo
	mknod(fifo_name, S_IFIFO | 0666, 0);
	//create the fifo_write
	mknod(fifo_name, S_IFIFO | 0666, 0);
	
	fd = open(fifo_name, O_RDONLY); //read only
	
	printf("We have a client! \n");
	
	do{
		if((num = read(fd,message,300)) == -1)
			perror("[server]Read from fifo error\n");
		else{
			char command[50]; //the command we send to the sockpair
			char msg[50];
			
			message[num] = '\0';
			printf("Requested command: %s\n",message);
			
			//THE LOGIN THING
			if(strncmp(message,"login",5)==0){
				//printf("Login works!\n");
				
				strcpy(command,"login");			
			
				bzero(msg,50);
				int j = 0;
				for(int i = 8; i < strlen(message) - 1; i++){
					//printf("%c\n",msg[i]);
					msg[j++] = message[i];
				}
				msg[strlen(msg)] = '\n';
			
			printf("Message recieved: %s\n", msg);	
			//printf("%ld\n", strlen(message));
			}
			
			//THE LOGOUT THING
			if(strncmp(message,"logout",6) == 0){
				strcpy(command,"logout");
			}
			
			//THE QUIT THING
			if(strncmp(message, "quit", 4) == 0){
				strcpy(command,"quit");
			}
			
			//printf("The command is %s\n", command);
			
			switch(pid = fork()){
				case -1:
					perror("Fork error");
				case 0:
					//printf("Child \n");
					if(strncmp(command, "login", 5) == 0){
						//printf("%s\n", msg);
						
						FILE *ptr;
						char UsernameToBeRead[50];
						ptr = fopen("file.txt", "r"); //read 
						
						if(NULL == ptr){ //error reading from file
							printf("file can't be open\n");
						}
						
						//going through the content of the file
						while(fgets(UsernameToBeRead, 50, ptr) != NULL)
						{
							//printf("%s", UsernameToBeRead);
							//printf("%s\n", msg);
							//printf("%ld\n", strlen(msg));
							
							if(strcmp(UsernameToBeRead, msg) == 0){
								printf("User logged in!\n");
								user_logged_in = 1; //we have 1 user
								
								//close(sockpair1[0]);
								if(write(sockpair1[1], &user_logged_in, sizeof(user_logged_in)) < 0){
									perror("[child]Error\n");
								}
								//close(sockpair1[1]);
								}
							
						}
						fclose(ptr);
						
					}
					
					if(strcmp(command, "logout") == 0){
						if(user_logged_in == 1){
							printf("User logout\n");
							user_logged_in = 0;
							username[0] = '\0';
							}
						else{
							printf("No user to logout\n");
						}
					}
					
					if(strcmp(command, "quit") == 0){
						printf("we kill the process\n");
						kill(getppid(), SIGKILL);
					}
					break;
					
				default: 
					//printf("Parent\n");
					//printf("%s\n", command);
					//printf("%d\n",user_logged_in);
					
					//close(sockpair1[1]);
					if(read(sockpair1[0], &user_logged_in, sizeof(user_logged_in)) < 0){
						perror("[parinte]Error\n");
					}
					
					if(user_logged_in == 1){
						for(int i = 0; i <= strlen(msg); i++){
							username[k++] = msg[i];
						
					}}
					
					//close(sockpair1[0]);
					break;
			}
				
		} 
				
	}while(num > 0);

	return 0;
}

/*
			if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair1) < 0) { 
    				perror("[server]Socketpair error \n"); 
        			exit(1);
        		}*/
        		
        		
/*			if ((pid = fork()) == -1){
				perror("[server]Fork error\n");
				exit(0);
			}
			
			if(pid){ //parent
				printf("Parent");
			}
			else { //child
				printf("chld");
				if(strncmp(command, "login",5) == 0){
					printf("the command is login");		
				}
			}
			*/
