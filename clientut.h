/**
   \file clientut.h 
   \author Giulio Muntoni
   \brief  dichiarazione funzioni di ausilio al client
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




/* =========================================================================================================*/
/** Delete NewLine
*/

char* deletenewline(char*s);





/* =========================================================================================================*/
/** Parsing
*/

char parsingMex(char* src, char** out);
