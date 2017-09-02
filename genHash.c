/**
   \file genHash.c
   \author Giulio Muntoni
   \brief  implementazione funzioni hash
 */

#include "genList.h"
#include "genHash.h"
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>



/** crea una tabella hash allocata dinamicamente
    \param size ampiezza della tabella
    \param compare funzione usata per confrontare due chiavi all'interno della tabella
    \param copyk funzione per copiare una chiave
    \param copyp funzione per copiare un payload
    \param hashfunction funzione hash (chiave,size della tabella)

    \retval NULL in caso di errori con \c errno impostata opportunamente
    \retval p (p!=NULL) puntatore alla nuova tabella allocata
*/
hashTable_t * new_hashTable (unsigned int size,  int (* compare) (void *, void *), void* (* copyk) (void *),void* (*copyp) (void*),unsigned int (*hashfunction)(void*,unsigned int)){

hashTable_t* t;
int i;

/* alloca una struct hashTable_t */
t = (hashTable_t*)malloc(sizeof(hashTable_t));
if (t == NULL){ perror("Malloc"); return NULL;} /* ritorna NULL se malloc fallisce */

/* alloca lo spazio necessario per tutto l'array di liste */
t->size = size;
t->table = (list_t **) malloc(size * sizeof(list_t *) );
if (t->table == NULL){ perror("Malloc");return NULL;} /* ritorna NULL se malloc fallisce */

/* setta tutte le liste inizialmente a null */
   for(i=0; i <t->size; i++) {
	      t->table[i]  = NULL;
	}


t->compare=compare;
t->copyk=copyk;
t->copyp=copyp;
t->hash = hashfunction;

return t;

}


/** funzione hash per key di tipo int
   \param key valore chiave
   \param size ampiezza della hash table

   \retval index posizione nella tabella
*/
unsigned int hash_int (void * key, unsigned int size){
int* key2 = (int*)key;
return (*key2)%size;

}


/** funzione hash per key di tipo string
   \param key valore chiave
   \param size ampiezza della hash table

   \retval index posizione nella tabella
*/
unsigned int hash_string (void * key, unsigned int size){ /* RICONTROLLARE SE L?HASH FUNZIAONA*/
int* key2;


key2 = (int*)key;

return (*key2)%size;

}


/** distrugge una tabella hash deallocando tutta la memoria occupata
    \param pt puntatore al puntatore della tabella da distruggere

    nota: mette a NULL il puntatore \c *pt
 */
void free_hashTable (hashTable_t ** pt){ /*CAMBIARE*/

int i;
i=0;

/* scorre l'array e dealloca tutte le liste presenti */
for(;i<(*pt)->size;i++){
	
	
	if ((*pt)->table[i] !=NULL){
	free_List(&(*pt)->table[i]);
	}
	}
/*dealloca la struct hashTable_t */
free((*pt)->table);
free((*pt));
(*pt)=NULL;

}

/** inserisce una nuova coppia (key, payload) nella hash table (se non c'e' gia')

    \param t la tabella cui aggiungere
    \param key la chiave
    \param payload l'informazione

    \retval -1 se si sono verificati errori (errno settato opportunamente)
    \retval 0 se l'inserimento \`e andato a buon fine

    SP ricordarsi di controllare se (in caso di errore) si lascia la situazione consistente o si fa casino nella lista ....
 */
int add_hashElement(hashTable_t * t,void * key, void* payload ){


int bol = 0;



/* Se nell'array non c'è nessuna collisione alloca una nuova lista nell'indice dato dalla hash e aggiunge l'elemento nella lista*/
if( t->table[t->hash(key,t->size)]==NULL) {  
	t->table[t->hash(key,t->size)] = new_List(t->compare,t->copyk,t->copyp);
	if( t->table[t->hash(key,t->size)]==NULL) { perror("Errore creazione lista"); return -1; } 
	return add_ListElement(t->table[t->hash(key,t->size)],key,payload);
	 
	}
else{
	bol = add_ListElement(t->table[t->hash(key,t->size)],key,payload);
	if (bol != 0){ return -1;}
	else return 0;
	}

}



/** cerca una chiave nella tabella e restituisce il payload per quella chiave
   \param t la tabella in cui aggiungere
   \param key la chiave da cercare
  
   \retval NULL in caso di errore (errno != 0) 
   \retval p puntatore a una \b copia del payload (alloca memoria)
 */
void * find_hashElement( hashTable_t * t,void * key ){

elem_t * a;

/* se l'indice non è NULL e trova l'elemento nella lista copia la sua payload e la ritorna tramite puntatore */
a = NULL;
if(t->table[t->hash(key,t->size)] !=NULL){
	a=find_ListElement( t->table[t->hash(key,t->size)],key);
		if (a != NULL){
				
				return t->copyp(a->payload);
				 }
				else return NULL;
}
else 
{
	
	return NULL;
}

}



/** elimina l'elemento di chiave (key) deallocando la memoria

    \param t puntatore alla lista
    \param key la chiave


    \retval -1 se si sono verificati errori (errno settato opportunamente)
    \retval 0 se l'esecuzione e' stata corretta

 */
int remove_hashElement( hashTable_t * t,void * key ){


/* se all'indice non è presente la lista non c'è niente da rimuovere */
if(t->table[t->hash(key,t->size)] ==NULL){ return -1; }/*CONTROLLARE VALORE*/


if(remove_ListElement(t->table[t->hash(key,t->size)], key)==-1){ return -1;}
	else return 0;
	
}


