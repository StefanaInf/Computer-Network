#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl;       //descriptorul intors de accept
    int id_user;
    int send_mess;
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int sd; //descriptorul de socket
int is_user_online;

thData *clients[100];
int nrClients = 0;

int main()
{
	struct sockaddr_in server;	// structura folosita de server
  	struct sockaddr_in from;	
	
	/* vector pentru thread */
    	pthread_t th[50];
    	int i = -1;

	/* crearea unui socket */
  	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    	{
      		perror ("[server]Eroare la socket().\n");
      		return errno;
    	}
    	
    	/* utilizarea optiunii SO_REUSEADDR */
  	int on=1;
  	setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	/* pregatirea structurilor de date */
  	bzero (&server, sizeof (server));
  	bzero (&from, sizeof (from));

	/* umplem structura folosita de server */
  	/* stabilirea familiei de socket-uri */
    	server.sin_family = AF_INET;	
  	/* acceptam orice adresa */
    	server.sin_addr.s_addr = htonl (INADDR_ANY);
  	/* utilizam un port utilizator */
    	server.sin_port = htons (PORT);

	/* atasam socketul */
  	if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    	{	
      		perror ("[server]Eroare la bind().\n");
      		return errno;
    	}
    	
    /* punem serverul sa asculte daca vin clienti sa se conecteze */
  	if (listen (sd, 5) == -1)
    	{
     		 perror ("[server]Eroare la listen().\n");
      		return errno;
      		
    	}

    /* servim in mod concurent clientii... */
	while(1){
		int client;
        thData *td; //parametru functia executata de thread
        int length = sizeof(from);

      	printf ("[server]Asteptam la portul %d...\n",PORT);
      	fflush (stdout);
      		
      	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      	client = accept (sd, (struct sockaddr *) &from, &length);

      	/* eroare la acceptarea conexiunii de la un client */
      	if (client < 0){
	  	    perror ("[server]Eroare la accept().\n");
	  	    continue;
	  	}	
	  	
	  	/* s-a realizat conexiunea */
	  	td=(struct thData*)malloc(sizeof(struct thData));	
		td->idThread=++i;
		td->cl=client;

        clients[i] = td;
        nrClients = i;

        //printf("aaaa%d bbb%d\n", clients[i], nrClients);

		pthread_create(&th[i], NULL, &treat, td);	
	}
}

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};

int edit_online(char username[], thData data)
{
    FILE *file;
    FILE *file1; //pt fisier temporar
    int line = 0;

    if((file = fopen("./users.txt", "a+")) == NULL) {
        perror("Eroare la fopen().\n");
        return 0;
    }

    //cautam in fisier userul pentru a ii afla id-ul
    char s[500];
    char nr[3];
    int k = 0;
    while(fgets(s, sizeof(s), file) != NULL) {
        if(strncmp(username, s, strlen(username)) == 0){
            char aux[100];
            strcpy(aux, s);
            //printf("%s\n", aux);
            for(int i = 0; i < strlen(aux); i++){
                if(aux[i] == ' ' && k == 0){
                    strcpy(nr, aux + i + 1);
                    k = 1;
                }
            }
        }
    }
    nr[2] = '\0';
    clients[data.idThread]->id_user = atoi(nr);
    //printf("This is %d\n", clients[data.idThread]->id_user);
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

int Shownotifications(int sd, thData data)
{
    //NUMELE FISIERULUI CARE CONTINE NOTIFICARILE UTILIZATORULUI
    char file_name[20];
    strcpy(file_name, "Notificari_");
    char id_current_user_string[10];
    int id_current_user = clients[data.idThread]->id_user;
    sprintf(id_current_user_string, "%d", id_current_user);
    strcat(file_name,id_current_user_string);
    
    printf("Fisierul care contine notificari: %s\n", file_name);

    int nr = countlines(file_name);
    printf("Numarul de linii a fisierului %s este %d\n", file_name, nr);

    if(nr != 0){
        if (write (sd, "yes", 50) <= 0)
    	{
      		perror ("[client]Eroare la write() spre server.\n");
      		return errno;
    	}

        //TRIMITEM NR DE LINII LA CLIENT
        char nr_linii[10];
        sprintf(nr_linii, "%d", nr);
        if (write (sd, nr_linii, 10) <= 0){
            perror ("[client]Eroare la write() spre server.\n");
      	    return errno;
        }

        strcat(file_name, ".txt");

        FILE *file;
        if((file = fopen(file_name, "r")) == NULL) { 
            perror("Eroare la fopen().\n");
            return 0;
        }

        char s[500];
        while(fgets(s, 500, file)!=NULL)
        {
            if(write(sd, s, sizeof(s))== -1)
            {
                perror("Eroare la write() spre server server");
                return 0;
            }
            else{bzero(s, 500); }
        }

        //DELETE THE CONTENT OF THE FILE
        fopen(file_name, "w");
        fclose(file);
    }
    else {
         if (write (sd, "no", 50) <= 0)
    	{
      		perror ("[client]Eroare la write() spre server.\n");
      		return errno;
    	}
    }
}

int Login(int sd, thData data)
{
    FILE *file;
    char username[100], password[8];
    char info[108]; //string in care punem si username si parola 
    int exists_username = 0;
    char user[100];

    bzero(username, sizeof(username));
    
    if (read (sd, username,sizeof(username)) < 0){
		perror ("Eroare la read() de la client.\n");
	}
    printf("%s\n", username);
    strcpy(info,username);
    strcpy(user,username);
   
    bzero(password, sizeof(password));

    if (read (sd, password, sizeof(password)) < 0){
		perror ("Eroare la read() de la client.\n");
	}
    password[8] = '\0';
    printf("%s\n", password);
    strcat(info, " ");
    strcat(info, password);  
    printf("%s\n",info);

    char s[500];
    if((file = fopen("./username.password.txt", "r")) == NULL) {
        perror("Eroare la fopen().\n");
        return 0;
    }
    
    while(fgets(s, sizeof(s), file) != NULL) {
        if(strncmp(info, s, strlen(info)) == 0){
            exists_username = 1;
        }
    }
    fclose(file);

    if(exists_username == 1){
        if (write (sd, "Login succesful", 50) <= 0)
    	{
      		perror ("[client]Eroare la write() spre server.\n");
      		return errno;
    	}

        edit_online(user, data);
        Shownotifications(sd, data);

    } else {
        if (write (sd, "Login unsuccesful, please register", 50) <= 0)
    	{
      		perror ("[client]Eroare la write() spre server.\n");
      		return errno;
    	}
    }
    is_user_online = 1;
}

int Register(int sd, thData data)
{
	FILE *file;
    char username[100], password[8];
    char info[108]; //string in care punem si username si parola 

    bzero(username, sizeof(username));
    bzero(password, sizeof(password));

    if (read (sd, username,sizeof(username)) < 0){
		perror ("Eroare la read() de la client.\n");
	}
    char aux[100];
    strcpy(aux, username);
    strcpy(info,username);

    if (read (sd, password, sizeof(password)) < 0){
		perror ("Eroare la read() de la client.\n");
	}
    password[8] = '\0';
    printf("%s\n", password);
    strcat(info, " ");
    strcat(info, password);  
    printf("%s\n",info);

    char s[500];

    if((file = fopen("./username.password.txt", "a")) == NULL) {
        perror("Eroare la fopen().\n");
        return 0;
    }
    
    fprintf(file, "%s", info);
    fclose(file);

    FILE *file1;
    if((file1 = fopen("./users.txt", "a+")) == NULL) {
        perror("Eroare la fopen().\n");
        return 0;
    }

    memset(aux+strlen(aux), ' ',1);
    int ch=0;
    int lines = 0;
    while ((ch = fgetc(file1)) != EOF)
    {
      if (ch == '\n')
      lines++;
    }
    fprintf(file1, "%s", aux);
    fprintf(file1, "%d", lines + 10);
    //memset(aux+ strlen(aux) + 3, ' ', 1);
    //fprintf(file1, "%s", " offline");
    fclose(file1);
}

int OwnFile(char message[], int sd, thData data)
{
    char buf[500];
    FILE *file;

    bzero(buf, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d.txt", clients[data.idThread]->id_user);
    if((file = fopen(buf, "a+")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }

    if(message[0] != '2'){
        fprintf(file, "%s\n", message);
    }
    fflush(file);
    fclose(file);
}

/*
int mesg(char file_name[])
{
    printf("aaaaaa");
    strcat(file_name, ".txt");
    //OPEN THE FILE SI TRIMITEM LINIILE CLIENTULUI
    FILE *file;
    if((file = fopen(file_name, "r")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }

    char s[500];
    while(fgets(s, 500, file)!=NULL)
    {
        if(write(sd, s, sizeof(s))== -1)
        {
            perror("Eroare la write() spre server server");
            return 0;
        }
        else{bzero(s, 500); }
    }
    fclose(file);
}*/

int sendMessageOnline(int sd, thData data, int nr_id, int j)
{
    printf("%s\n","User online");

    if (write (sd, "User online!", 50) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
      	return errno;
    }

    printf("%d_%d\n",clients[data.idThread]->id_user,nr_id); //name of the file

    char buf1[500], buf2[500];
    FILE *file1, *file2;
   
    bzero(buf1, sizeof(buf1));
    snprintf(buf1, sizeof(buf1), "%d_%d.txt", clients[data.idThread]->id_user, nr_id);
    if((file1 = fopen(buf1, "a+")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }

    bzero(buf2, sizeof(buf2));
    snprintf(buf2, sizeof(buf2), "%d_%d.txt", nr_id, clients[data.idThread]->id_user);
    if((file2 = fopen(buf2, "a+")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }

    char id_current_user_string[10];
    char id_other_user_string[10];
    int id_current_user = clients[data.idThread]->id_user;
    sprintf(id_current_user_string, "%d", id_current_user);
    sprintf(id_other_user_string, "%d", nr_id);
    strcat(id_current_user_string,"_");
    strcat(id_current_user_string, id_other_user_string);
    
    //VARIABILA PENTRU NUMARAREA LINIILOR
    int nr = countlines(id_current_user_string);
    printf("Nr de linii: %d\n", nr);
    //TRIMITEM CLIENTULUI NR DE LINII
    char nr_linii[10];
    sprintf(nr_linii, "%d", nr);
    if (write (sd, nr_linii, 10) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
      	return errno;
    }
    nr++;

    char message[500];
    int chatting = 0; 
    while(chatting == 0){
        printf("%s\n", "Waiting for a message from client.");

        //CITIM MESAJUL DE LA CLIENT
        bzero(message, sizeof(message));
        if (read (sd, message,sizeof(message)) < 0){
		    perror ("Eroare la read() de la client.\n");
	    }
        printf("Mesajul primit: %s\n", message);
        //SCRIEM IN FISIERUL PERSONAL AL USERULUI

        if(message[0] == '#'){        
            chatting = 1;
            printf("%s\n","Clientul doreste sa termine conversatia");
        }else{
            if(message != NULL){
                if(message[0] != ' '){
                    OwnFile(message, sd, data);
                    if(message[0] == '-'){
                        fprintf(file1, "nr.%d: ", nr);
                        fprintf(file2, "nr.%d: ", nr);

                        fprintf(file1, "%d replies to ", clients[data.idThread]->id_user);
                        fprintf(file2, "%d replies to ", clients[data.idThread]->id_user);                        
                }else{
                    fprintf(file1, "nr.%d: ", nr);
                    fprintf(file2, "nr.%d: ", nr);

                    fprintf(file1, "%d: ", clients[data.idThread]->id_user);
                    fprintf(file2, "%d: ", clients[data.idThread]->id_user);
                }
                fprintf(file1, "%s\n", message);
                fprintf(file2, "%s\n", message);
                fflush(file1); fflush(file2); 
                }
                //fclose(file1); fclose(file2);

                //TRIMITEM CLIENTULUI NR DE LINII
                int nr2 = countlines(id_current_user_string);
                char nr_linii2[10];
                sprintf(nr_linii2, "%d", nr2);
                if (write (sd, nr_linii2, 10) <= 0){
                    perror ("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                printf("%s\n", id_current_user_string);
                //mesg(id_current_user_string);
                char file_name[500];
                strcpy(file_name ,id_current_user_string);
                strcat(file_name, ".txt");
                FILE *file;
                if((file = fopen(file_name, "r")) == NULL) { 
                    perror("Eroare la fopen().\n");
                    return 0;
                }

                char s[500];
                while(fgets(s, 500, file)!=NULL)
                {
                    if(write(sd, s, sizeof(s))== -1)
                    {
                        perror("Eroare la write() spre server server");
                        return 0;
                    }
                    else{bzero(s, 500); }
                }
                fclose(file);
            }
        }
        nr++;
    }
    fclose(file1); 
    fclose(file2);
}


int Notificari(char message[], int sd, thData data, int id)
{
    char buf[500];
    FILE *file;
   
    bzero(buf, sizeof(buf));
    snprintf(buf, sizeof(buf), "Notificari_%d.txt", id);
    if((file = fopen(buf, "a+")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }
    if(message[0] != '#'){
        printf("%d...%s\n", id, message);
        fprintf(file, "%d: ", id);
        fprintf(file, "%s\n", message);
    }

    fflush(file);
    fclose(file);
}

int sendMessageOffline(int sd, thData data, int nr_id)
{
    printf("%s\n", "User offline");

    if (write (sd, "User offline!", 50) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
      	return errno;
    }

    char message[500];
    int chatting = 0;
    while(chatting == 0){
        printf("%s\n", "Waiting for a message from client.");

         if(strncmp(message, "#", 1) == 0) {
            chatting = 1;
        }else{

            bzero(message, sizeof(message));
            if (read (sd, message,sizeof(message)) < 0){
                perror ("Eroare la read() de la client.\n");
            }

            Notificari(message, sd, data, nr_id);
        }
    }
}

int SendMessage(int sd, thData data)
{
    int is_online = 0;

    //CITIM USERUL CU CARE VREM SA VORBIM DE LA CLIENT
    char id[50];
    bzero(id, sizeof(id));
    if (read (sd, id,sizeof(id)) < 0){
		perror ("Eroare la read() de la client.\n");
	}
    printf("User selected: %s\n", id);

    //CAUTAM USERUL IN LISTA DE UTLIZATORI ONLINE
    int j;
    for(int i = 0; i <= nrClients; i++){
        if(clients[i]->id_user == atoi(id) && clients[data.idThread]->id_user != atoi(id)){
            is_online = 1;
            j = i;
        }
    }

    if(is_online == 1){
        sendMessageOnline(sd, data, atoi(id), j);
    }
    else{
        sendMessageOffline(sd, data, atoi(id));
    }
}

int SeeHistory(int sd, thData data)
{
    //CITIM DE LA CLIENT ID CLIENTULUI CU CARE VREA SA VORBEASCA
    char ras[10];
    bzero(ras, sizeof(ras));
    if (read (sd, ras,sizeof(ras)) < 0){
		perror ("Eroare la read() de la client.\n");
	}
    printf("See history with: %s\n", ras);

    char id_current_user_string[10];
    int id_current_user = clients[data.idThread]->id_user;
    sprintf(id_current_user_string, "%d", id_current_user);
    strcat(id_current_user_string,"_");
    strcat(id_current_user_string,ras);
    printf("%s\n", id_current_user_string);

    int nr = countlines(id_current_user_string);
    printf("Numarul de linii din fisierul %s este de %d\n", id_current_user_string, nr);

    //TRIMITEM NR DE LINII LA CLIENT
    char nr_linii[10];
    sprintf(nr_linii, "%d", nr);
    if (write (sd, nr_linii, 10) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
      	return errno;
    }

    strcat(id_current_user_string, ".txt");

    //OPEN THE FILE SI TRIMITEM LINIILE CLIENTULUI
    FILE *file;
    if((file = fopen(id_current_user_string, "r")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }

    char s[500];
    while(fgets(s, 500, file)!=NULL)
    {
        if(write(sd, s, sizeof(s))== -1)
        {
            perror("Eroare la write() spre server server");
            return 0;
        }
        else{bzero(s, 500); }
    }
}

int OwnHistory(int sd, thData data)
{
    char id_current_user_string[10];
    int id_current_user = clients[data.idThread]->id_user;
    sprintf(id_current_user_string, "%d", id_current_user);

    int nr = countlines(id_current_user_string);
    printf("Numarul de linii din fisierul %s este de %d\n", id_current_user_string, nr);

    //TRIMITEM NR DE LINII LA CLIENT
    char nr_linii[10];
    sprintf(nr_linii, "%d", nr);
    if (write (sd, nr_linii, 10) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
      	return errno;
    }

    strcat(id_current_user_string, ".txt");

    //OPEN THE FILE SI TRIMITEM LINIILE CLIENTULUI
    FILE *file;
    if((file = fopen(id_current_user_string, "r")) == NULL) { 
        perror("Eroare la fopen().\n");
        return 0;
    }

    char s[500];
    while(fgets(s, 500, file)!=NULL)
    {
        if(write(sd, s, sizeof(s))== -1)
        {
            perror("Eroare la write() spre server server");
            return 0;
        }
        else{bzero(s, 500); }
    }
}

int Logout(int sd, thData data)
{
    clients[data.idThread]->id_user = 0;
}

int SeeUsers(int sd, thData data)
{
    printf("%s\n", "Comanda selectata see users");
    char filename[10] = "users.txt";
    int nr = countlines("users");
    printf("Numarul de linii din fisierul %s este de %d\n", filename,nr);

    //TRIMITEM NR DE LINII LA CLIENT
    char nr_linii[10];
    sprintf(nr_linii, "%d", nr);
    if (write (sd, nr_linii, 10) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
      	return errno;
    }

    FILE *file; 
    if((file = fopen("./users.txt", "r")) == NULL) {
        perror("Eroare la fopen().\n");
        return 0;
    }
    
    char s[500];
    while(fgets(s, 500, file)!=NULL)
    {
        if(write(sd, s, sizeof(s))== -1)
        {
            perror("Eroare la write() de la server");
            return 0;
        }
        else{bzero(s, 500); }
    }
}

void raspunde(void *arg)
{
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	char raspuns[50];
		
	while(1){
		bzero(raspuns, sizeof(raspuns));

		if (read (tdL.cl, raspuns,sizeof(raspuns)) < 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");

			}
		
		printf("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, raspuns);
		
		if(strncmp(raspuns,"login",5)==0){
            Login(tdL.cl, tdL);
        }

        if (strncmp(raspuns,"register",8) == 0){
            Register(tdL.cl, tdL);
        }

        if (strncmp(raspuns,"exit",4) == 0){
            break;
        }

        if(is_user_online == 1){

            //AFISAREA MESAJELOR PRIMITE CAT TIMP A FOST OFFLINE

            if(strcmp(raspuns, "1") == 0)
                SeeUsers(tdL.cl, tdL);

            if(strcmp(raspuns, "2") == 0) {
                SendMessage(tdL.cl,tdL);
            }

            if(strcmp(raspuns, "3") == 0) {
                SeeHistory(tdL.cl,tdL);
            }
            
            if(strcmp(raspuns, "4") == 0) {
                OwnHistory(tdL.cl,tdL);
            }

            if(strcmp(raspuns, "6") == 0) {
                Logout(tdL.cl,tdL);
            }
            
        }   

    }
}
