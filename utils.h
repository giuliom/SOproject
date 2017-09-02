/**  \file utils.h
 *    \author Giulio Muntoni 442752
 *  \brief  macro di utilità per il progetto e il debug
 *
*/

#ifndef UTILS_H
#define UTILS_H




/** define per scegliere lo stream in cui stampare i messaggi di debug*/
#define STREAM stdout

/**System Call exception -1 with perror and exit */
#define EC_SC(s,m) \
	if((s) == -1) { perror(m); exit(errno); }

/** System Call exception -1 with return -1  */
#define EC_RT(s,m) \
	if((s) == -1) { perror(m); return -1; }

/** System call exception !=0 return -1 */
#define EC_TH(s,m) \
	if((s) != 0) { perror(m); return -1; }

/** System call exception NULL return NULL */
#define EC_NLL(s,m) \
	if((s) == NULL) { perror(m); return NULL; }

/** System call exception NULL return -1 */
#define EC_NL(s,m) \
	if((s) == NULL) { perror(m); return -1; }


	#ifdef DEBUG
	
		/** Stampano rispettivamente un messaggio, un intero o una stringa sullo stream STREAM*/

		#define debmex(msg) fprintf(STREAM, "\nDEBUG: %s\n",msg);
	
		#define debint(msg,val) fprintf(STREAM,"\nDEBUG: %s %d\n",msg,val);

		#define debstr(msg,val) fprintf(STREAM,"\nDEBUG: %s %s\n",msg,val);

	#else

		#define debmex(msg);
		#define debint(msg,val)
		#define debstr(msg,val)
		

	#endif

#endif











