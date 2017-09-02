/**
   \file serverut.c
   \author Giulio Muntoni
   \brief  Implementazione funzioni di utilità per il server
 */


#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<pthread.h>
#include"genHash.h"
#include"genList.h"
#include"comsock.h"
#include"serverut.h"
#include"utils.h"



/* Mutex Pipe */
static pthread_mutex_t pipemtx = PTHREAD_MUTEX_INITIALIZER;


/* =========================================================================================================*/
/** Compare  
*/

int compare_int(void *a, void *b) {
    int *_a, *_b;
    _a = (int *) a;
    _b = (int *) b;
    return ((*_a) - (*_b));
}


int compare_string(void *a, void *b) {
    char *_a, *_b;
    _a = (char *) a;
    _b = (char *) b;
    return strcmp(_a,_b);
}


/* =========================================================================================================*/
/** Copy
*/

void * copy_int(void *a) {
  int * _a;

  if ( ( _a = malloc(sizeof(int) ) ) == NULL ) return NULL;

  *_a = * (int * ) a;

  return (void *) _a;
}


void * copy_string(void * a) {
  char * _a;

  if ( ( _a = strdup(( char * ) a ) ) == NULL ) return NULL;

  return (void *) _a;
}



/* =========================================================================================================*/
/** Print List
*/
void print_List(list_t * q){
elem_t* p;
for(p = q->head;p !=NULL;p=p->next){

int* pay = (int*)p->payload;

printf(" %s:%d ", (char*)p->key,*pay);
}
}




/* =========================================================================================================*/
/** Print hashTable
*/
void print_hashTable(hashTable_t * hs){

list_t* p;
int i =0;
for(p = hs->table[i];i < hs->size;p = hs->table[i])
{
	printf("HASH: %d \n", i);
	print_List(p);
	printf("\n");
	i++;
	
}
printf("\n\n");
}



/* =========================================================================================================*/
/** Controlla la validità di stringa (nome utente o file)
* \param s la stringa da controllare
* \param mode valori accettati 0 e 1 utilizzato per far cmpiere due funzioni diverse alla funzione
* ed evitare di crearne altre. Scelta discutibile
*/
int stringcheck(char* s,int mode){

if (strlen(s) >256 || strlen(s) <1) return -1;


/* per i file */
if (mode ==0)
{
	for(; *s != '\0'; s++)
	{
		if (*s =='_') return -1;
	}
}

/* per gli utenti */
if (mode ==1)
{
	for(; *s != '\0'; s++)
	{
		if ( (*s >='a' && *s<='z')  || (*s >='A' && *s<='Z') || (*s >='0' && *s<='9') ) {}
		else
			{ return -1;}
	
	
	}
}
return 0;

}




/* =========================================================================================================*/
/** Crea una hasTable_t 
* aggiungendo tutti gli utenti autorizzati e settando il loro payload con il valore -1 che indica che sono disconnessi
* \param size é parametro della funzione hash che ne specifica l'immagine 
* \param argv contiene il nome del file che contiene la lista di utenti
* \retval il puntatore all'hastable in caso di successo o NULL in caso di errore
*/
hashTable_t * createHashTable(int size, char* argv[]){

/** FILE DESCRIPTOR USERLIST*/
FILE *fd = NULL;

/** payload */
int pay = -1;
int* load = &pay;
int a;

/** Buffer */
char* buf;


hashTable_t* hs;

buf = (char*)malloc(sizeof(char)*256);

/* controlla il nome del file */
if (stringcheck(argv[1],0) <0) { perror("File Errato\n"); return NULL;}


/** hashTable Creation */

if(( hs = new_hashTable(size,compare_string,copy_string,copy_int,hash_string))==NULL){printf("Error createHash"); return NULL;}


/** APERTURA FILE */
if((fd = fopen(argv[1],"r"))== NULL) { perror("File Errato\n"); return NULL;}


/** Aggiunge i nomi letti nella hashtable e setta il payload a -1 segnalando che sono disconnessi */
while ((a = fscanf(fd,"%s", buf)) >0)
{
	if (stringcheck(buf,1)==-1){printf("File utenti Corrotto\n");return NULL;}
	if(add_hashElement(hs,buf, load )==-1){printf("add fail"); return NULL;}
	
}

/** chiude il file con la lista utenti */
fclose(fd);

return hs;
}


/* =========================================================================================================*/
/** Gestisce il Login di un cLient
* \param hs la hastable con gli utenti ammessi
* \param buf nome utente che richiede la connessione
* \retval 0 in caso di connessione accettata o -1
*/
int clientLogin(hashTable_t* hs, char* buf ){


int* pay;

if (stringcheck(buf,1)==-1){printf("Nome Errato\n");return -1;}

pay = find_hashElement(hs,buf);


if (pay != NULL)
{
	if (*pay == -1)
	{
	
	
	return 0;
	}
	
}
return -1;
}



/* =========================================================================================================*/
/** Gestisce MSG_LIST
* \param hs la hastable con gli utenti ammessi
* \param s messaggio da inviare
* \retval 0 successo
* \warning: valgrind segnala "uninitiliased bytes" alla prima esecuzione della funzione dopo la creazione della hashtable
*/
int msgList(hashTable_t* hs, char** s){

char*t = NULL; 
char*u = NULL;
list_t* p;
elem_t *q;
int i =0;
int slen=0;
int* c;

(*s) = (char*)malloc(sizeof(char)*6+1);
strcpy((*s),"[LIST]");

/** Scorre tutta la hashtable e aggiunge al messaggio gli utenti connessi */
for(p = hs->table[i];i < hs->size;p = hs->table[i])
{
	
  if (p!=NULL)
  {
	for(q = p->head;q !=NULL;q=q->next)
	{
		if (q->payload !=NULL)
		{
			c= q->payload;
		
			if ( *c !=-1)
			{
	
			t = (char*)malloc(slen+sizeof(char)*strlen(q->key)+1+1);
			if ( u != NULL)
			{		
				strcpy(t,u);
				strcat(t," ");
				free(u); /* la t precedente */
			}			
			else {strcpy(t," ");}

			strcat(t,q->key);
			u=t;
			slen= strlen(u);
			}		
		}
	}
  }  
	
i++; 
}

t = (*s);
(*s) = (char*)malloc(sizeof(char)*6+sizeof(char)*slen+1);
strcpy((*s),t);
free(t); /* prima allocazione di s */
strcat((*s),u);




return 0;
}




/* =========================================================================================================*/
/** Gestisce MSG_TO_ONE 
* utilizza lo stesso algoritmo di parseMessage in clientut.c potrebbe esserne atratta una funzione
* \param hs la hastable con gli utenti ammessi
* \param s messaggio da inviare
* \param mitt nome del client che invia il messaggio
* \param pfd pipe per salvare il log
* \retval una copia del valore del socket del destinatario
*/
int* msgToOne(hashTable_t* hs,char** s,char* mitt,int pfd){

char* dest,*q;
char* p;
int i =0;
int j=0;
int* con;
char* a = ": utente non connesso";

message_t* msg;


/** Ricava il nome del destinatario */
for(p = *s; *p !=' ' && *p !='\0' && *p !='\n'; p++)
{
	i++;
}

EC_NLL((dest = (char*)malloc(sizeof(char)*i+1)),"Errore malloc");

q = *s;


for(p = dest; j<i; j++)
{
	*p = *q;
	p++;
	q++;
	
}

 *p= '\0';


for(p = *s; *p !=' ' && *p !='\0' && *p !='\n'; p++){}

/** cerca la socket di comunicazione con il destinatario */
con = find_hashElement(hs,dest); /* ALLOCA NUOVO PAYLOAD!*/



/** se il destinatario é connesso invia il messaggio altrimenti restituisce
* al mittente un messaggio di errore*/
if (con !=NULL && (*con) !=-1)
{
	/** Messaggio di ritorno */
	(*s) = (char*)malloc(sizeof(char)*strlen(p)+sizeof(char)*strlen(mitt)+3);
	strcpy((*s),"[");
	strcat((*s),mitt);
	strcat((*s),"]");
	strcat((*s),p);
	
	/** Messaggio di log */
	p++;
	
	msg =(message_t*)malloc(sizeof(message_t));
	msg->type = MSG_OK;
	msg->buffer = (char*)malloc(sizeof(char)*strlen(p)+sizeof(char)*strlen(mitt)+sizeof(char)*strlen(dest)+3);
	strcpy(msg->buffer,mitt);
	strcat(msg->buffer,":");
	strcat(msg->buffer,dest);
	strcat(msg->buffer,":");
	strcat(msg->buffer,p);
	msg->length = strlen(msg->buffer)+1;

	
	/* SEZIONE CRITICA: invia messaggio via pipe */
	pthread_mutex_lock(&pipemtx);
	sendMessage(pfd,msg);
	pthread_mutex_unlock(&pipemtx);
	
	free(msg->buffer);
	free(msg);

}
else
{
	(*s) = (char*)malloc(sizeof(char)*strlen(a)+sizeof(char)*strlen(dest)+8+1);
	strcpy((*s),"[ERROR] ");
	strcat((*s),dest);
	strcat((*s),a);
	if (con !=NULL)free(con);
	con = NULL;
}



 return con;

}



/* =========================================================================================================*/
/** Gestisce MSG_BCAST STESSA STORIA DI CLIENTUT PARSING!!!
* \param hs la hastable con gli utenti ammessi
* \param buf messaggio da inviare
* \param mitt nome del client che invia il messaggio
* \param pfd pipe per salvare il log
* \warning: valgrind segnala "uninitiliased bytes" alla prima esecuzione della funzione dopo la creazione della hashtable
*/
int msgBcast(hashTable_t* hs,char* buf,char* mitt,int pfd){


list_t* p;
elem_t *q;
int i =0;
int* c;
message_t* msg,*mex;

/** Alloca e prepara i messaggio per i destinatari e per la pipe */
char* bmsg = (char*)malloc(sizeof(char)*strlen(mitt)+sizeof(char)*strlen(buf)+10+1);
strcpy(bmsg,"[BCAST]");
strcat(bmsg,"[");
strcat(bmsg,mitt);
strcat(bmsg,"] ");
strcat(bmsg,buf);
msg = (message_t*)malloc(sizeof(message_t));
mex = (message_t*)malloc(sizeof(message_t));
msg->type = 'B';
mex->type = 'B';
msg->length = strlen(bmsg)+1;
msg->buffer = bmsg;

/** scorre la hashtable e invia il messaggio agli utenti connessi */
for(p = hs->table[i];i < hs->size;p = hs->table[i])
{
   if (p!=NULL)
  {
	for(q = p->head;q !=NULL;q=q->next)
	{
		 c= q->payload;
		if ( *c !=-1)
		{
			/* Messaggio bcast */
			sendMessage(*c,	msg);

			/* Messaggio di LOG */
			mex->buffer = (char*)malloc(sizeof(char)*strlen(buf)+sizeof(char)*strlen(mitt)+sizeof(char)*strlen(q->key)+3);		
			strcpy(mex->buffer,mitt);
			strcat(mex->buffer,":");
			strcat(mex->buffer,q->key);
			strcat(mex->buffer,":");
			strcat(mex->buffer,buf);
			mex->length = strlen(mex->buffer)+1;
			

			/* SEZIONE CRITICA: invia messaggio via pipe */
			pthread_mutex_lock(&pipemtx);
			sendMessage(pfd,mex);
			pthread_mutex_unlock(&pipemtx);
			
			free(mex->buffer);
		}	
	}
  }

i++;
}

free(msg->buffer);
free(msg);
free(mex);

return 0;
}


/* =========================================================================================================*/
/** Carica il file di opzioni
* \param hashvalue il valore con cui eseguire il calcolo dell'hash per gli elementi della hashtable
* \param sockfile la posizione in cui creare il socket di collegamento con il server
* \param numero massimo di utenti connessi permesso
* \param il file di opzioni
* \retval 0 success ; -1 errore
*/

int optparse(int* hashvalue, char** sockfile, int* maxusers, FILE* opt)
{

EC_RT(fscanf(opt,"%d", hashvalue),"Errore lettura file di opzioni\n");

(*sockfile) = (char*)malloc(sizeof(char)*256);
EC_RT(fscanf(opt,"%s", (*sockfile)),"Errore lettura file di opzioni\n");

EC_RT(fscanf(opt,"%d", maxusers),"Errore lettura file di opzioni\n");

return 0;
}
