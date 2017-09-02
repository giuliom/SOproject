/**
   \file genList.c
   \author Giulio Muntoni
   \brief  implementazione funzioni lista generica
 */

#include<stdio.h>
#include<stdlib.h>
#include"genList.h"
#include<errno.h>




/** crea una lista generica
    \param compare funzione usata per confrontare due chiavi 
    \param copyk funzione usata per copiare una chiave
    \param copyp funzione usata per copiare un payload

    \retval NULL in caso di errori con \c errno impostata opportunamente
    \retval p (p!=NULL) puntatore alla nuova lista
*/
list_t * new_List(int (* compare) (void *, void *),void * (* copyk) (void *),void * (* copyp) (void*)){

list_t* lista;
lista = (list_t*)malloc(sizeof( list_t));
if (lista == NULL){  perror("Errore Malloc");return NULL;} /* ritorna NULL se malloc fallisce*/

lista->head= NULL;
lista->compare=compare;
lista->copyk=copyk;
lista-> copyp=copyp;

return lista;
}


/** distrugge una lista deallocando tutta la memoria occupata
    \param pt puntatore al puntatore della lista da distruggere

    nota: mette a NULL il puntatore \c *t
 */
void free_List (list_t ** pt){

elem_t* p0 = (*pt)->head; 
elem_t* q = NULL;

if ((*pt) == NULL || pt == NULL){ perror("Puntatore list_t nullo");return ;} /* se il puntatore è nullo ritorna null */


/*scorre gli elem_t dela lista pt e cancella ogni elemento allocato*/
while (p0 != NULL)
	{
	q = p0;
	p0 = p0->next;
	free(q->key);
	free(q->payload);
	free(q);
	}
/* cancella la struttura list_t*/
free((*pt));
(*pt)=NULL;

}


/** inserisce una nuova coppia (key, payload) in testa alla lista, 
    sia key che payload devono essere copiate nel nuovo elemento della lista.
    Nella lista \b non sono permesse chiavi replicate

    \param t puntatore alla lista
    \param key la chiave
    \param payload l'informazione

    \retval -1 se si sono verificati errori (errno settato opportunamente)
    \retval 0 se l'inserimento \`e andato a buon fine

 */
int add_ListElement(list_t * t,void * key, void* payload){ 


elem_t* p;
elem_t* new;
/* Crea il nuovo elemento*/
new  = (elem_t*)malloc(sizeof( elem_t));
if (new == NULL){ perror("Errore Malloc"); return -1;} /* ritorna -1 se malloc fallisce*/

new->key = t->copyk(key); 
new->payload = t->copyp(payload);



/* Scorre la lista per vedere se l'elemento è già presente*/
for (p = t->head; p !=NULL; p = p->next){

	if (t->compare(p->key,key)==0){ /*dealloca il nuovo elemento new perchè non va inserito*/
							free(new->key);
							free(new->payload);
							free(new);
							 return -1;}
}

new->next = t->head;
t->head = new; 

return 0;
}


/** elimina l'elemento di chiave (key) deallocando la memoria

    \param t puntatore alla lista
    \param key la chiave


    \retval -1 se si sono verificati errori (errno settato opportunamente)
    \retval 0 se l'esecuzione e' stata corretta

 */
int remove_ListElement(list_t * t,void * key){

elem_t* p;
elem_t* q =NULL;
/* scorre la lista cercando l'elemento da rimuovere e tenendo traccia dell' elem_t predecessore */
for (p = t->head; p !=NULL; p = p->next)
	{

if (t->compare(p->key,key)==0){ 
/* se dobbiamo rimuovere in testa */
if ( p==t->head){ 
	t->head = p->next;
	free(p->key);
	free(p->payload);
	
	free(p);
	return 0;}
else {
	q->next = p->next;
	free(p->key);
	free(p->payload);
	free(p);
	return 0;}	
}
q =p;
}
return -1;
}


/** cerca l'elemento di chiave (key)

    \param t puntatore alla lista
    \param key la chiave


    \retval NULL se l'elemento non c'e'
    \retval p puntatore all'elemento trovato (puntatore interno alla lista non alloca memoria)

 */

elem_t * find_ListElement(list_t * t,void * key){

elem_t* p;


for (p = t->head; p !=NULL; p = p->next){

	if (t->compare(p->key,key)==0){
		return p;}
}
return NULL;
}




