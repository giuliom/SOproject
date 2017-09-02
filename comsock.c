/**  \file 
 *    \author Giulio Muntoni 442752
 *  \brief implementazione libreria di comunicazione socket AF_UNIX
 *
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include <string.h>
#include"comsock.h"
#include"utils.h"

#define SOCKNAME "./tmp/msgsock"



/** Crea una socket AF_UNIX
 *  \param  path pathname della socket
 *
 *  \retval s    il file descriptor della socket  (s>0)
 *  \retval SNAMETOOLONG se il nome del socket eccede UNIX_PATH_MAX
 *  \retval -1   in altri casi di errore (sets errno)
 *
 *  in caso di errore ripristina la situazione inziale: rimuove eventuali socket create e chiude eventuali file descriptor rimasti aperti
 */
int createServerChannel(char* path){


int fd_skt;


/** Crea il sockaddr */
struct sockaddr_un sa;

fd_skt = 0;

/** Controlla se il nome é troppo lungo */
if (strlen(path) > UNIX_PATH_MAX) { return SNAMETOOLONG; }


strncpy(sa.sun_path, path,UNIX_PATH_MAX);
sa.sun_family=AF_UNIX;



/** Crea il socket */
EC_RT(fd_skt = socket(AF_UNIX,SOCK_STREAM,0), "Socket Creation Error\n"); 



/** Assegna l'indirizzo saal socket */
EC_RT(bind(fd_skt,(struct sockaddr *)&sa,sizeof(sa)),"Errore Bind\n");



/** Listen: pronto a ricevere */
EC_RT(listen(fd_skt, SOMAXCONN), "errore listen");



return fd_skt;

}


/* ============================================================================================================================================ */

/** Chiude una socket
 *   \param s file descriptor della socket
 *
 *   \retval 0  se tutto ok, 
 *   \retval -1  se errore (sets errno)
 */
int closeSocket(int s){



shutdown(s, SHUT_RDWR);
close(s);


return 0;

}

/* ============================================================================================================================================ */

/** accetta una connessione da parte di un client
 *  \param  s socket su cui ci mettiamo in attesa di accettare la connessione
 *
 *  \retval  c il descrittore della socket su cui siamo connessi
 *  \retval  -1 in casi di errore (sets errno)
 */
int acceptConnection(int s){


int fd_c = 0;

EC_RT(fd_c = accept(s,NULL,0),"AccConn Error");

return fd_c;


}


/* ============================================================================================================================================ */


/** legge un messaggio dalla socket
 *  \param  sc  file descriptor della socket
 *  \param msg  struttura che conterra' il messagio letto 
 *		(deve essere allocata all'esterno della funzione,
 *		tranne il campo buffer)
 *
 *  \retval lung  lunghezza del buffer letto, se OK 
 *  \retval SEOF  se il peer ha chiuso la connessione 
 *                   (non ci sono piu' scrittori sulla socket)
 *  \retval  -1    in tutti gl ialtri casi di errore (sets errno)
 *      
 */
int receiveMessage(int sc, message_t * msg){

int a = 0;

EC_RT(a = read(sc,&(msg->type),sizeof(char)), "ReceiveMEssage");

if (a  == 0) return SEOF;

EC_RT(read(sc,&(msg->length),sizeof(int)), "ReceiveMEssage");



if (msg->length != 0)
{
msg->buffer = (char*)malloc(sizeof(char)*msg->length);
EC_RT(read(sc,msg->buffer,msg->length), "ReceiveMEssage");
}


return msg->length;

return 0;

}


/* ============================================================================================================================================ */


/** scrive un messaggio sulla socketprintf(" RICEVE: %c \n", msg->type);
 *   \param  sc file descriptor della socket
 *   \param msg struttura che contiene il messaggio da scrivere 
 *   
 *   \retval  n    il numero di caratteri inviati (se scrittura OK)
 *   \retval  SEOF se il peer ha chiuso la connessione 
 *                   (non ci sono piu' lettori sulla socket) 
 *   \retval -1   in tutti gl ialtri casi di errore (sets errno)
 
 */

int sendMessage(int sc, message_t *msg){


int n =0;


EC_SC(n += write(sc,&(msg->type),sizeof(char)), "sendMessage");
EC_SC(n += write(sc,&(msg->length),sizeof(int)), "sendMessage");

if (msg->length != 0)
{
	EC_SC(n += write(sc,msg->buffer,msg->length), "sendMessage");
}


return n;

}


/* ============================================================================================================================================ */


/** crea una connessione all socket del server. In caso di errore funzione tenta NTRIALCONN volte la connessione (a distanza di 1 secondo l'una dall'altra) prima di ritornare errore.
 *   \param  path  nome del socket su cui il server accetta le connessioni
 *   
 *   \return ilsocket se la connessione ha successo
 *   \retval SNAMETOOLONG se il nome del socket eccede UNIX_PATH_MAX
 *   \retval -1 negli altri casi di errore (sets errno)
 *
 *  in caso di errore ripristina la situazione inziale: rimuove eventuali socket create e chiude eventuali file descriptor rimasti aperti
 */
int openConnection(char* path){

struct sockaddr_un sa;
int fc_skt;

int i  = NTRIALCONN;


/** Controlla se il nome é troppo lungo */
if (strlen(path) > UNIX_PATH_MAX) { return SNAMETOOLONG; }

/** Crea il sockaddr */
strncpy(sa.sun_path, path,UNIX_PATH_MAX);
sa.sun_family=AF_UNIX;

fc_skt = 0;

/** Crea il socket */
EC_RT((fc_skt = socket(AF_UNIX,SOCK_STREAM,0)), "Socket Creation Error: OpenConnection"); 



	while ((connect(fc_skt,(struct sockaddr *)&sa,sizeof(sa))==-1) && i>0)
	{	
	sleep(1);
	i--;
	}


if (fc_skt <0) return -1;




return fc_skt;

}














	
