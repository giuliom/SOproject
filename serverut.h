/**
   \file serverut.h
   \author Giulio Muntoni
   \brief  dichiarazione funzioni di ausilio al server
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
#include"utils.h"











/* =========================================================================================================*/
/** Compare  
*/

int compare_int(void *a, void *b) ;


int compare_string(void *a, void *b) ;

/* =========================================================================================================*/
/** Copy
*/

void * copy_int(void *a) ;


void * copy_string(void * a);


/* =========================================================================================================*/
/** Print
*/

void print_List(list_t * q);


void print_hashTable(hashTable_t * q);


/* =========================================================================================================*/
/** Controlla una stringa
*/
int stringcheck(char* s,int mode);


/* =========================================================================================================*/
/** Crea una hasTable_t 
*/

hashTable_t * createHashTable(int size, char* argv[]);


/* =========================================================================================================*/
/** Gestisce il Login di un cLient
*/
int clientLogin(hashTable_t* hs, char* buf);


/* =========================================================================================================*/
/** Gestisce MSG_LIST
*/

int msgList(hashTable_t* hs,char** s);


/* =========================================================================================================*/
/** Gestisce MSG_TO_ONE
*/

int* msgToOne(hashTable_t* hs,char** s,char* mitt,int pfd);


/* =========================================================================================================*/
/** Gestisce MSG_BCAST
*/
int msgBcast(hashTable_t* hs,char* buf,char* mitt,int pfd);


/* =========================================================================================================*/
/** Carica il file di opzioni
*/

int optparse(int* hashvalue, char** sockfile, int* maxusers, FILE* opt);
