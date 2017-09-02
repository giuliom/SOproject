/**
   \file clientut.c 
   \author Giulio Muntoni 442752
   \brief  implementazione funzioni di ausilio al client
 */


#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include"genHash.h"
#include"genList.h"
#include"comsock.h"
#include"clientut.h"
#include"utils.h"



/* =========================================================================================================*/
/** Delete NewLine - alloca memoria
* \param s la stringa a cui va rimosso il carattere finale newline
* \retval una nuova stringa senza newline
*/

char* deletenewline(char*s){


char* fin, *p,*q;

fin = (char*)malloc(sizeof(char)*strlen(s));

p = fin;
q = s;

/** copia la stringa finoal carattere \n */
for(; *q !='\n' && *q !='\0';)
{
	*p = *q;
	p++;
	q++;
	
}

/** aggiunge il carattere di terminazione stringa*/
*p = '\0';

return fin;
}





/* =========================================================================================================*/
/** Parsing Messaggio
* analizza il tipo di input
* \param src stringa ricevuta tramite input
* \param out risultato del parsing
* \retval tipo del messaggio
*/
char parsingMex(char* src, char** out){

char* p, *q;
/** il tipo di comando segnalato da % seguito da una stringa */
char* prefix = NULL;
int i=0;
int j = 0;


/** Caso MSG_BAST 
* se il primo carattere non é % significa che si tratta di un messaggio broadcast
*/
if (*src != '%') 
{
(*out) = deletenewline(src);
return MSG_BCAST;
}

/** conto la lunghezza del comando fino al primo spazio, identificando il prefisso */
for(p = src; *p !=' ' && *p !='\0' && *p !='\n'; p++)
{
	i++;
}


prefix = (char*)malloc(sizeof(char)*i+1);
q = src;

/** copio il prefisso */
for(p = prefix; j<i; j++)
{
	*p = *q;
	p++;
	q++;
	
}

 *p= '\0';

/** scorro il puntatore fino all'inizio del messaggio vero e proprio */
for(p = src; *p !=' ' && *p !='\0' && *p !='\n'; p++){}
p++;





if (strcmp(prefix,"%LIST")==0)
{
	free(prefix);
	(*out) = NULL;
	return MSG_LIST;
}

if (strcmp(prefix,"%EXIT")==0)
{
	free(prefix);
	return MSG_EXIT;	
}	

if (strcmp(prefix,"%ONE")==0)
{
	free(prefix);
	(*out) = deletenewline(p); /* alloca nuova stringa = out  */
	return MSG_TO_ONE;
}


/** Caso in cui l'input ha una sintassi errata */
free(prefix);
printf("Comandi disponibili Percentuale seguito da:  LIST , EXIT e  ONE utente messaggio  oppure  messaggio  per l'invio broadcast \n");
return MSG_ERROR;

}
