/**
   \file msgcli.c
   \author Giulio Muntoni 442752
   \brief  Implementazione Client
 */

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<sys/select.h>
#include <string.h>
#include<pthread.h>
#include<signal.h>
#include"comsock.h"
#include"genHash.h"
#include"genList.h"
#include"serverut.h"
#include"clientut.h"
#include"utils.h"


#define SOCKNAME "./tmp/msgsock"
#define LOGPATH "./log"
#define EOF (-1)



/** Variabili Globali */

/** flag ricezione segnale */
volatile sig_atomic_t sigflag = 0;






/**=====================================Thread In===================================
* thread  READER che legge i messaggi dalla socket
 */

void* in(void* arg){


int* fd = (int*)arg;


message_t* msg;
EC_NLL((msg =(message_t*)malloc(sizeof(message_t))),"errore malloc msg");
msg->type = MSG_BCAST;

/** Il MSG_OK viene inviato dal server solo in seguito ad un MSG_EXIT */
while ( msg->type != MSG_OK )
{
		/**Alloca il buffer!*/
		if (receiveMessage(*fd,msg) != SEOF) 
		{ 	
			if (msg->length >0){fprintf (stdout,"%s\n",msg->buffer);
			free(msg->buffer);}
		}
		else
		{
			msg->type = MSG_OK;
		}		
	
	
}


/** Ripulire ambiente */
fflush(stdout);
free(msg);

debmex("Thread READER chiuso");

return 0;

}



/** ====================================Thread Out======================================
* thread WRITER che invia i messaggi al server
*/

void* out(void* arg){

int* fd = (int*)arg;

/** Stringa per input da stdin */
char in[1000];
char* s = NULL;



message_t* msg;
EC_NLL((msg =(message_t*)malloc(sizeof(message_t))),"errore malloc msg");
msg->length = 0;
msg->buffer = NULL;

/** Finché non é stato ricevuto un segnale aspetta l'input con fgets. Parsa l'input
* invia il messaggio con modalità diverse a seconda del tipo e se il tipo non è MSG_EXIT 
* continua il ciclo 
*/

while (sigflag==0)
{
	
		
	if (fgets(in, 999, stdin) != NULL)
	{ 
	/** Fa il parsing dell'input restituendo la stringa s */
	msg->type = parsingMex(in,&s);

	if ( msg->type != MSG_ERROR && msg->type != MSG_EXIT)
		{
			if (s != NULL)
			{
			
				msg->length = strlen(s)+1; 	
				EC_NLL((msg->buffer = (char*)malloc(sizeof(char)*msg->length)),"Errore Malloc");
				strcpy(msg->buffer,s);		
				sendMessage(*fd,msg);				
				free(s);
				free(msg->buffer);
			}		
			else
			{
				msg->length = 0;
				msg->buffer = NULL;
				sendMessage(*fd,msg);
			}		
			
			
			
			
		}
	if(msg->type == MSG_EXIT) {sigflag =1; }

	}
	else
		{sigflag =1; }
}

/** risveglia il main */
kill(getpid(),SIGINT);

/** notifica al server l'uscita del client */
msg->type = MSG_EXIT;
msg->length = 0;
msg->buffer = NULL;
sendMessage(*fd,msg); 



/** Ripulire ambiente */
free(msg);

debmex("Thread WRITER chiuso");

return 0;

}



/** ******************************** M A I N ***************************************************************************** */




int main(int argc, char *argv[]){

/** Gestione Segnali */
sigset_t setalarm;
int sig = 0;


char* name = argv[1];
int fd;
/** thread WRITER */
pthread_t tidwr;
/** thread READER */
pthread_t tidrd;

message_t* msg;


/** Controlla gli argomenti del programma*/
if (argc >2 || argc<2){perror("Nome Errato\n"); return -1;}
if (argv[1] == NULL) {perror("Nome Errato\n"); return -1;}
if (stringcheck(argv[1],0)==-1){perror("Nome Errato\n"); return -1;}


/* Gestione SIGALRM */
EC_RT(sigemptyset(&setalarm),"Sigemptyset");
EC_RT(sigaddset(&setalarm,SIGALRM),"Sigaddset");
EC_RT(sigaddset(&setalarm,SIGINT),"Sigaddset");
EC_RT(sigaddset(&setalarm,SIGTERM),"Sigaddset");
EC_RT(pthread_sigmask(SIG_SETMASK, &setalarm, NULL),"Sigmask");

/** Apre la connessione con il server */
EC_RT((fd = openConnection(SOCKNAME)),"Errore connessione Server");


EC_NL((msg =(message_t*)malloc(sizeof(message_t))),"errore malloc msg");
EC_NL((msg->buffer = (char*)malloc(sizeof(char)*strlen(name)+1)),"Errore malloc msg->buffer");

msg->type = MSG_CONNECT;
msg->length = strlen(name)+1;
strcpy(msg->buffer,name);

/** invia una richiesta di connessione al server e aspetta la risposta */
sendMessage(fd,msg);
free(msg->buffer);
if (receiveMessage(fd,msg) == SEOF) { perror("Connesione persa"); return -1;}


if (msg->type == MSG_OK) 
{
	debmex("Connessione Stabilita\n");
	
	EC_TH(pthread_create(&tidwr, NULL, &out,&fd),"Errore pthread_create");

	EC_TH(pthread_create(&tidrd, NULL, &in,&fd),"Errore pthread_create");

	
}

if (msg->type == MSG_ERROR) {printf("[ERROR] Utente non Autorizzato\n"); return -1;}



/** attende un segnale */
sigwait(&setalarm,&sig); 

debint("Segnale arrivato", sig);


/** Esce gentilmente notificando l'uscita al server*/
msg->type=MSG_EXIT;
msg->length = 0;
sendMessage(fd,msg);

/** Aspetta la chiusura del thread READER */
EC_TH(pthread_join(tidrd,NULL),"pthread join"); 


/** Termina il thread WRITER che rimane bloccato su fgets */
pthread_cancel(tidwr);


free(msg->buffer);
free(msg);

debmex("Uscita Client...");

exit(EXIT_SUCCESS);

return 0;
}
