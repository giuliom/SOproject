/**
   \file msgserv.c
   \author Giulio Muntoni 442752
   \brief  Implementazione Server
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
#include"utils.h"



#define SOCKNAME "./tmp/msgsock"
#define LOGPATH "./log"
#define EOF (-1)



/**======================= Struct Clientinfo ==========================
* \struct struttura contente informazioni da passare ai thread WORKER
*/
typedef struct {

/** Nome del Client */
char* name;
/** Pipe */
int pfd[2];


} clientInfo;



/*======================== VARIABILI GLOBALI =========================*/

/** \var file Desriptor Socket */
int skt;

/** \var file logfile */
FILE* logfile;

/** \var Tabella Hash */
hashTable_t* hs;

/** \var Utenti online */
int online = 0;

/** Mutex */
static pthread_mutex_t hsmutx = PTHREAD_MUTEX_INITIALIZER;





/**======================= SIGNAL HANDLER =============================*/
static void sigHandler_pc(){

printf("\nSEGNALE RICEVUTO\n");
closeSocket(skt);
}




/**====================== THREAD WRITER ==============================
* thread che si occupa della scrittura del log ricevendo i segnali dai clients
* tramite una pipe
*/
void* writer(void* arg){

/** per comodità utilizza la stessa struttura dei worker */
clientInfo* inf = (clientInfo*)arg;
message_t* msg;

EC_NLL((msg =(message_t*)malloc(sizeof(message_t))),"errore malloc msg");

/** finché riceve i messaggi e ci sono thread scrittori sulla pipe */
while (receiveMessage(inf->pfd[0],msg) !=-2)
{
	
	fprintf(logfile,"%s\n",msg->buffer);
	free(msg->buffer);
}

/*chiude il file di log - fflush chiamata alla chisura del thread writer */
fclose(logfile);
free(inf);
free(msg);

debmex("Chiudo il thread writer");
return NULL;

}




/**====================== THREAD WORKER ==============================*/


void* worker(void* arg){

/** Socket di connessione */
int* con = NULL; 
int* b;
char* ls = NULL;
int pay = -1;
int* load = &pay;
clientInfo* inf = (clientInfo*)arg;
message_t* mex;

/*alloca memoria, copia il payload*/
con = find_hashElement(hs,inf->name);

EC_NLL((mex =(message_t*)malloc(sizeof(message_t))),"errore malloc msg-worker");

printf("%s connesso \n",inf->name);


/** Ricezione e invio messaggi con il client */
while (receiveMessage((*con), mex) !=-1)
{
	
	
   switch (mex->type)
   {

	/** MSG_LIST */	
	case MSG_LIST:
	{

		msgList(hs,&ls );
		mex->length = strlen(ls)+1;	
		mex->buffer = ls;
		mex->type = MSG_LIST;
		sendMessage((*con),mex);
		free(ls);
		break;	
	}
	

	/** MSG_TO_ONE */
	case MSG_TO_ONE:
	{
		ls = mex->buffer;
		b =msgToOne(hs, &ls,inf->name,inf->pfd[1]);
		if (b != NULL)
		{
			mex->length = strlen(ls)+1;	
			free(mex->buffer);
			mex->buffer = ls;
			mex->type = MSG_TO_ONE;
			sendMessage((*b),mex);
			free(ls);
		}
		else
		{
		free(mex->buffer);
		mex->buffer = ls;
		mex->length = strlen(ls)+1;
		mex->type = MSG_ERROR;
		sendMessage((*con),mex);
		free(ls);
		}
		break;
	}
	
	
	/** MSG_BCAST */
	case MSG_BCAST:
	{
		msgBcast(hs,mex->buffer,inf->name,inf->pfd[1]);
		break;
	}
	

	/** MSG_EXIT */
	case MSG_EXIT:
	{	
		mex->type = MSG_OK;
		mex->length = 0;
		mex->buffer = NULL;
		sendMessage((*con),mex);
		printf("%s disconnesso dal server\n",inf->name);
		
		
		
		/** SEZIONE CRITICA: setta il payload a -1 nella hashtable poiché il clint si é disconnesso*/
		pthread_mutex_lock(&hsmutx);
		remove_hashElement(hs,inf->name);
		add_hashElement(hs,inf->name,load);
		online--;		
		pthread_mutex_unlock(&hsmutx);
		
		
		fflush(NULL);
		free(mex);
		free(inf->name);	
		free(inf);
		pthread_exit(0);
		break;
	}
	
  }

}

/** Terminazione a causa del Server */	
printf("%s Thread chiuso\n",inf->name);


fflush(NULL);
free(mex);
free(inf->name);
free(inf);
pthread_exit(0);
	
	

return NULL;

}




/** ******************************** M A I N ***************************************************************************** */




int main(int argc, char *argv[]){


/** Gestione Segnali */
sigset_t set;
struct sigaction s, sp;

int* a;
int i = 0;

/** variabili configurazione server */
int hashvalue = 3;
int maxusers = 10;
char* sockfile = SOCKNAME;
FILE* optfile;

list_t* p;
elem_t *q;


/** file descriptor Socket Connesso */
int con;

/** File Descriptor Pipe  0 Lettura - 1 scrittura */
int pfd[2];

/** Tid del thread  */
pthread_t tid;
pthread_t tid_wr;

/**Message */
message_t* msg;


/**Client Info */
clientInfo* inf, *inf2;

EC_NL((msg = (message_t*)malloc(sizeof(message_t))),"Errore malloc");


/** Gestione segnali */
EC_RT(sigfillset(&set),"Errore gestore segnali");
EC_RT(sigprocmask(SIG_SETMASK,&set,NULL),"Errore gestore segnali");
memset(&s,sizeof(s),0);
bzero(&s, sizeof(s));
bzero(&sp, sizeof(sp));
s.sa_handler = sigHandler_pc;
sp.sa_handler = SIG_IGN;
EC_RT(sigaction(SIGINT,&s,NULL),"Errore gestore segnali");
EC_RT(sigaction(SIGTERM,&s,NULL),"Errore gestore segnali");
EC_RT(sigaction(SIGPIPE,&sp,NULL),"Errore gestore segnali");
EC_RT(sigemptyset(&set),"Errore gestore segnali");
EC_RT(sigprocmask(SIG_SETMASK,&set,NULL),"Errore gestore segnali");


if (argc <3 || argc >4) {printf("Argomenti Errati\n"); return -1; }

/** Carica e parsa file opzioni */
if ((optfile = fopen(argv[3],"r"))==NULL) {printf("File configurazione non trovato\n");}
else 
{ 
optparse(&hashvalue, &sockfile,&maxusers, optfile);
printf("hashvalue: %d\nsockfile:%s\n maxusers: %d\n",hashvalue,sockfile,maxusers);
fclose(optfile);
}

/** Crea la Tabella hash e inserisce i nomi letti nel file argv[1] */
EC_NL((hs = createHashTable(hashvalue, argv)), "hashtable non creata");


/** Crea la struct ClientInfo */
EC_NL((inf = (clientInfo*)malloc(sizeof(clientInfo))),"Errore malloc");

/** crea/tronca il file di log */
EC_NL((logfile = fopen(argv[2],"w")),"Errore file log");



/** Crea il socket */
unlink(sockfile);
skt = createServerChannel(sockfile);



/** Crea Thread Writer */
EC_RT(pipe(pfd),"Errore creazione pipe");
inf->pfd[0] = pfd[0];
inf->pfd[1] = pfd[1];
EC_TH(pthread_create(&tid_wr, NULL, &writer,inf),"Errore pthread_create");



printf("SERVER OPERATIVO \n\n");



/** MAIN LOOP */
while ((con = acceptConnection(skt)) !=-1)
{

	
   if (receiveMessage(con, msg) != SEOF)
   {
	
	

	/** Verfica le autorizzazioni alla connessione */
	if(msg->type == MSG_CONNECT ) 
	{
		if (clientLogin(hs,msg->buffer)==0 && online < maxusers)
		{
			/* SEZIONE CRITICA:aggiunge il client alla hastable hs */
			pthread_mutex_lock(&hsmutx);
			remove_hashElement(hs, msg->buffer);
			add_hashElement(hs,msg->buffer,&con); 
			online++;
			pthread_mutex_unlock(&hsmutx);


			EC_NL((inf2 = (clientInfo*)malloc(sizeof(clientInfo))),"Errore malloc");
			inf2->pfd[0] = pfd[0];
			inf2->pfd[1] = pfd[1];
			
			inf2->name = (char*)malloc(sizeof(char)*msg->length);
			strcpy(inf2->name,msg->buffer);
			
			debint("Utenti Connessi",online);
					
	
			/** Crea thread worker */	
			EC_TH(pthread_create(&tid, NULL, &worker,inf2),"Errore pthread_create");
			msg->type = MSG_OK;
			sendMessage(con,msg);
			
			
		}
		else
		{
			msg->type = MSG_ERROR;
			sendMessage(con,msg);
			closeSocket(con);
		}
	}
	free(msg->buffer);
    }
}





/* chiude i socket di comunicazione con i client scorrendo la hashtable*/
pthread_mutex_lock(&hsmutx);
for(p = hs->table[i];i < hs->size;p = hs->table[i])
{
  if (p!=NULL)
  {	
	for(q = p->head;q !=NULL;q=q->next)
	{
		a = (int*)q->payload;
		
	
	if (*a !=-1)
	{
		printf("Chiudo il thread:%s %d\n",(char*)q->key,*a);
		closeSocket(*a);
		*a = -1;
	}

	}
  }

i++;
}
pthread_mutex_unlock(&hsmutx);


close(pfd[0]);
close(pfd[1]);

/** attende il thread writer che salva il log */
EC_TH(pthread_join(tid_wr,NULL),"pthread join");

/** cancella l'hash table*/
free_hashTable(&hs);
free(msg);

/**Cancella il file msgsock */
unlink(sockfile);

printf("Shutdown Server \n");

return 0;

}
