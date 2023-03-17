#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server */
int port;

int is_user_online;

void print_menu()
{
    printf("%s\n", "Welcome! Select a command: ");
    printf("%s\n", "*Login");
    printf("%s\n", "*Register");
    printf("%s\n", "*Exit");
}

int menu2;
void print_menu2()
{
    printf("%s\n", "Select command from: ");
    printf("%s\n", "[1] See users");
    printf("%s\n", "[2] Send message to user");
    printf("%s\n", "[3] History with user");
    printf("%s\n", "[4] Own history");
    printf("%s\n", "[5] Menu");
    printf("%s\n", "[6] Logout");
}

int countlines(char *filename)
{
    char buf[500];
    FILE *file;
    bzero(buf, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s.txt", filename);
    if((file = fopen(buf, "a+")) == NULL) {
        perror("Eroare la fopen().\n");
        return 0;
    }

    char msg[1000];
    int nr = 0;
    while(fgets(msg, 1000, file) != NULL) nr++;
    return nr;
}

int main(int argc, char *argv[])
{
	char command[50];
    int nr_command;

	int sd;  //descriptorul de socket
	struct sockaddr_in server; //structura folosita pentru conectare

	/* exista toate argumentele in linia de comanda? */
  	if (argc != 3)
    	{
     	 	printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      		return -1;
    	}

	/* stabilim portul */
	port = atoi(argv[2]);

	/* cream socketul */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        	perror("Eroare la socket().\n");
        	return errno;
    	}
	
	/* umplem structura folosita pentru realizarea conexiunii cu serverul */
    	/* familia socket-ului */
    	server.sin_family = AF_INET;
    	/* adresa IP a serverului */
    	server.sin_addr.s_addr = inet_addr(argv[1]);
    	/* portul de conectare */
    	server.sin_port = htons(port);

	/* ne conectam la server */
  	if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1){
      		perror ("[client]Eroare la connect().\n");
      		return errno;
    	}
	
    print_menu();

	while(1){
        /*read command from keyboard*/
        bzero(command, sizeof(command));
        scanf("%s", command);
  		
  		/* trimiterea mesajului la server */
		if (write (sd, command, 50) <= 0){
      			perror ("[client]Eroare la write() spre server.\n");
      			return errno;
    	}

        if(strncmp(command,"login",5)==0){
            printf ("Your command: login\n");

            char username[100];
            char password[8];

            printf ("Username: ");  
            bzero(username, sizeof(username));
            scanf("%s", username);

            if (write (sd, username, sizeof(username)) <= 0){
      			perror ("[client]Eroare la write() spre server.\n");
      			return errno;
    	    }

            printf ("Password: ");  
            bzero(password, sizeof(password));
            scanf("%s", password);

            if (write (sd, password, sizeof(password)) <= 0){
      			perror ("[client]Eroare la write() spre server.\n");
      			return errno;
    	    }

            char msg[50];
            if (read (sd, msg, 50) < 0){
      		    perror ("[client]Eroare la read() de la server.\n");
      		    return errno;
      	    }
            printf ("%s\n", msg);
            if(strcmp(msg, "Login succesful") == 0){
                is_user_online = 1;
            }

            //CITIM DE LA SERVER DACA USERUL ARE NOTIFICARI
            char mg[50];
            if (read (sd, mg, 50) < 0){
      		    perror ("[client]Eroare la read() de la server.\n");
      		    return errno;
      	    }
            printf("Does user have notifications: %s\n", mg);

            if(strncmp(mg, "yes", 3) == 0){
                //CITIM NR DE LINII DE LA SERVER
                char nr_str[10];
                if (read (sd, nr_str, 10) < 0){
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                //printf("Nr de linii: %s\n", nr_str);

                //AFISAM LINIILE PRIMITE DE LA SERVER    
                char buffer[500];
                for(int i = 0; i < atoi(nr_str); i++)
                {
                    if (read (sd, buffer, 500) < 0){
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    printf("%s", buffer);
                    bzero(buffer, 500);
                }
            }
            else{
                //nada
            }

        }

        if(strncmp(command,"register",8)==0){
            printf ("Your command: register\n");

            char username[100];
            char password[8];

            printf ("Username: ");  
            bzero(username, sizeof(username));
            scanf("%s", username);

            if (write (sd, username, sizeof(username)) <= 0){
      			perror ("[client]Eroare la write() spre server.\n");
      			return errno;
    	    }

            printf ("Password: ");  
            bzero(password, sizeof(password));
            scanf("%s", password);

            if (write (sd, password, sizeof(password)) <= 0){
      			perror ("[client]Eroare la write() spre server.\n");
      			return errno;
    	    }
            printf ("%s\n", "Registered!");
        }     

        //PRINT MENU
        if(strcmp(command,"5")==0){
           print_menu2();
        }

        if(is_user_online == 1){
            //@PRINT MENU
            if(menu2 == 0){
                print_menu2();
                menu2 = 1;
            }

            //@SEE USERS
            if(strcmp(command,"1") == 0) {
                printf("%s\n", "Users and their ids:");

                //CITIM NR DE LINII DE LA SERVER
                char nr_str[10];
                if (read (sd, nr_str, 10) < 0){
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                }
                
                char buffer[500];
                //printf("Nr de linii: %s\n", nr_str);
                for(int i = 0; i < atoi(nr_str); i++)
                {
                    if (read (sd, buffer, 500) < 0){
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    printf("%s", buffer);
                    bzero(buffer, 500);
                }
            }

            //@HISTORY WITH OTHER USER
            if(strcmp(command,"3")==0){
                printf("%s", "See history with a user. Please input ID: ");
                
                char id_h[10];
                bzero(id_h, sizeof(id_h));
                //CITIIM DE LA TASTAURA ID-UL USERULUI
                scanf("%s", id_h);

                //TRIMITEM ID-UL CU UTILIZATORULUI CU CARE VREM SA VORBIM
                if (write (sd, id_h, sizeof(id_h)) <= 0){
      			    perror ("[client]Eroare la write() spre server.\n");
      			    return errno;
    	        }

                //CITIM NR DE LINII DE LA SERVER
                char nr_str[10];
                if (read (sd, nr_str, 10) < 0){
      		        perror ("[client]Eroare la read() de la server.\n");
      		        return errno;
      	        }
                //printf("Nr de linii de la server: %s\n", nr_str);

                //AFISAM LINIILE PRIMITE DE LA SERVER    
                char buffer[500];
                for(int i = 0; i < atoi(nr_str); i++)
                {
                    if (read (sd, buffer, 500) < 0){
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    printf("%s", buffer);
                    bzero(buffer, 500);
                }
            }

            //@OWN HISTORY
            if(strcmp(command,"4")==0){
                printf("%s\n","Own history");

                //CITIM NR DE LINII DE LA SERVER
                char nr_str[10];
                if (read (sd, nr_str, 10) < 0){
      		        perror ("[client]Eroare la read() de la server.\n");
      		        return errno;
      	        }
                //printf("Nr de linii de la server: %s\n", nr_str);
            
                //AFISAM LINIILE PRIMITE DE LA SERVER    
                char buffer[500];
                for(int i = 0; i < atoi(nr_str); i++)
                {
                    if (read (sd, buffer, 500) < 0){
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    printf("%s", buffer);
                    bzero(buffer, 500);
                }
            }

            //@LOGOUT
            if(strcmp(command,"6")==0){
                printf("%s\n","User loged out!");
            }    

            //@CHATTING
            if(strcmp(command,"2")==0){
                printf("Choose to chat with (please type the ID): ");

                //CITIM SI TRIMITEM ID-UL CU CARE VREM SA VORBIM
                char id[50]; 
                bzero(id, sizeof(id));
                scanf("%s",id);
                if (write (sd, id, sizeof(id)) <= 0){
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                //CITIM MESAJUL ONLINE/OFFLINE
                char msg[50];
                if (read (sd, msg, 50) < 0){
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
      	        }
                printf ("%s\n", msg);

                if(strncmp(msg,"User online!",13) == 0){
                    char message[500];
                    bzero(message, sizeof(message));

                    printf("%s\n","====Chat====");
                    //CITIM MESAJUL DE LA TASTAURA
                    //gets(message, 500, stdin);

                    //CITIM NR DE LINII DE LA SERVER
                    char nr_str[10];
                    if (read (sd, nr_str, 10) < 0){
                        perror ("[client]Eroare la read() de la server.\n");
                        return errno;
                    }
                    //printf("Nr de linii de la server: %s\n", nr_str);
                
                    gets(message, 500, stdin);
                    int ok = 1;
                    while(ok == 1){
                        char message[500];
                        //CITIM MESAJUL DE LA TASTATURA SI TRIMITEM SERVERULUI
                        printf("you: ");
                        gets(message, 500, stdin);
                        //printf("%s\n", message);
                        //if(message[0] != ' '){
                            if (write (sd, message, sizeof(message)) <= 0){
                                perror ("[client]Eroare la write() spre server.\n");
                                return errno;
                            }
                        //}
                        if(strncmp(message, "#", 1) == 0) {
                            ok = 0;
                        }
                        else{
                            //CITIM NR DE LINII DE LA SERVER
                            char nr_str2[10];
                            if (read (sd, nr_str2, 10) < 0){
                                perror ("[client]Eroare la read() de la server.\n");
                                return errno;
                            }
                            //printf("Nr de linii de la server: %s\n", nr_str);

                            system("clear"); 
                            //AFISAM LINIILE PRIMITE DE LA SERVER    
                            char buffer[500];
                            for(int i = 0; i < atoi(nr_str2); i++)
                            {
                                if (read (sd, buffer, 500) < 0){
                                    perror ("[client]Eroare la read() de la server.\n");
                                    return errno;
                                }
                                printf("%s", buffer);
                                bzero(buffer, 500);
                            }
                        }
                    }
                }else{
                    int ok = 0;
                    gets(message, 500, stdin);
                    while(ok == 0){
                        char message[500];
                        //CITIM MESAJUL DE LA TASTATURA SI TRIMITEM SERVERULUI
                        printf("you: ");
                        gets(message, 500, stdin);
                        //printf("%s\n", message);
                        if (write (sd, message, sizeof(message)) <= 0){
                            perror ("[client]Eroare la write() spre server.\n");
                            return errno;
                        } 
                        
                        if(strncmp(message, "#", 1) == 0) {
                            ok = 1;
                            break;
                        }
                    }
                }
            }
        }
	}
	/* inchidem conexiunea, am terminat */
  	close (sd);
}