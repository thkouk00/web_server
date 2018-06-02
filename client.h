#ifndef _CLIENTH_
#define _CLIENTH_

#include <stdio.h>
#include <sys/types.h> 		/* sockets */
#include <sys/socket.h> 	/* sockets */
#include <sys/stat.h>		/* mkdir */
#include <sys/select.h>		/* select() */
#include <sys/wait.h>		/* waitpid */
#include <netinet/in.h> 	/* internet sockets */
#include <unistd.h> 		/* read, write, close */
#include <netdb.h> 			/* gethostbyaddr */
#include <stdlib.h> 		/* exit */
#include <string.h> 		/* strlen */
#include <dirent.h>			/* directory */	
#include <pthread.h>		/* threads - compile with -pthread at end */
#include <sys/timeb.h>		/* timeb struct */
#include "check_response.h"	/* take code , length of response */
#include "buflist.h"		/* url_queue structure */

extern fd_set set, readfds;
extern short int shtdwn_flag, exit_flag;
extern char *save_dir;
// must be set to NULL  
extern url_queue *queue; 
extern url_queue *checked_urls;
extern int count, working_threads;
extern int served_pages, total_bytes;			//for stats 
extern pthread_t *tid;
extern pthread_mutex_t mtx , clock_mtx , stat_mtx;
extern pthread_cond_t cond_nonempty;
extern struct timeb start,end;

#define REQUEST "GET %s HTTP/1.1\r\n"\
				"Host: %s\r\n"\
				"\r\n"

typedef struct args_struct{
	int fd;
	int port;
	char *host;
	// struct sockaddr *serverptr;
	url_queue *queue;
} args_struct;

void* worker_client(void*);
void* commands_client(void*);

#endif