/**
   \file genList.h
   \author lso10
   \brief  prototipi e strutture dati lista generica
 */

#ifndef __GENLIST__H
#define __GENLIST__H

/** nodo della lista generica

Notare che sia la chiave (\b key) che l'informazione (\b payload) sono dei puntatori generici

*/

typedef struct elem {
  /** chiave */
  void * key;
  /** informazione */
  void * payload;
  /** puntatore elemento successivo */
  struct elem * next;
} elem_t; 


/**
   Lista generica

   compare -- permette di confrontare due chiavi, ritorna 0 se sono uguali ed un valore diverso da 0 altrimenti
   copyk,copyp -- creano una copia di una chiave o un payload (allocando la memoria necessaria) e ritornano il puntatore alla copia (se tutto e' andato bene) o NULL (se si e' verificato un errore)
 */
typedef struct {
  /** la testa della lista */
  elem_t * head;
  /** la funzione per confrontare due chiavi */
  int (* compare) (void *, void *);
  /** la funzione per copiare una chiave */
  void * (* copyk) (void *);
  /** la funzione per copiare un payload*/
  void * (* copyp) (void *);
} list_t;

/** FUNZIONI DA IMPLEMENTARE */

/** crea una lista generica
    \param compare funzione usata per confrontare due chiavi 
    \param copyk funzione usata per copiare una chiave
    \param copyp funzione usata per copiare un payload

    \retval NULL in caso di errori con \c errno impostata opportunamente
    \retval p (p!=NULL) puntatore alla nuova lista
*/
list_t * new_List(int (* compare) (void *, void *),void* (* copyk) (void *),void* (*copyp) (void*));

/** distrugge una lista deallocando tutta la memoria occupata
    \param pt puntatore al puntatore della lista da distruggere

    nota: mette a NULL il puntatore \c *t
 */
void free_List (list_t ** pt);

/** inserisce una nuova coppia (key, payload) in testa alla lista, 
    sia key che payload devono essere copiate nel nuovo elemento della lista.
    Nella lista \b non sono permesse chiavi replicate

    \param t puntatore alla lista
    \param key la chiave
    \param payload l'informazione

    \retval -1 se si sono verificati errori (errno settato opportunamente)
    \retval 0 se l'inserimento \`e andato a buon fine

 */
int add_ListElement(list_t * t,void * key, void* payload);

/** elimina l'elemento di chiave (key) deallocando la memoria

    \param t puntatore alla lista
    \param key la chiave


    \retval -1 se si sono verificati errori (errno settato opportunamente)
    \retval 0 se l'esecuzione e' stata corretta

 */
int remove_ListElement(list_t * t,void * key);

/** cerca l'elemento di chiave (key)

    \param t puntatore alla lista
    \param key la chiave


    \retval NULL se l'elemento non c'e'
    \retval p puntatore all'elemento trovato (puntatore interno alla lista non alloca memoria)

 */
elem_t * find_ListElement(list_t * t,void * key);

#endif
