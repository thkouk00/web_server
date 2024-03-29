#ifndef _SERVERH_
#define _SERVERH_
#include <stdio.h>
#include <stdlib.h>			//malloc , exit
#include <sys/wait.h>		//sockets
#include <sys/types.h>		//sockets
#include <sys/select.h>		//select()
#include <netinet/in.h>		//htons,htonl,ntohs,ntohl
#include <netdb.h>			//gethostbyaddr, gethostbyname
#include <sys/socket.h>		//all socket functions
#include <unistd.h>			//access
#include <ctype.h>			//toupper
#include <signal.h>			//signal
#include <pthread.h>		//threads - compile with -pthread at end
#include <string.h>			//strerror , error printing for threads
#include <time.h>			//strftime
#include <sys/timeb.h>		//timeb struct
#include <sys/sendfile.h>	//more efficient way to send file than read-write , no use in here
#include "buflist.h"		//queue to hold fds
#include "valid_request.h"	//check if GET request is valid

#define REQFOUND "HTTP/1.1 200 OK\r\n"\
				"Date: %s\r\n"\
				"Server: myhttpd/1.0.0 (Ubuntu64)\r\n"\
				"Content-Length: %d\r\n"\
				"Content-Type: text/html\r\n"\
				"Connection: Closed\r\n"\
				"\r\n"

#define REQNOTFOUND "HTTP/1.1 404 Not Found\r\n"\
					"Date: %s\r\n"\
					"Server: myhttpd/1.0.0 (Ubuntu64)\r\n"\
					"Content-Length: %ld\r\n"\
					"Content-Type: text/html\r\n"\
					"Connection: Closed\r\n"\
					"\r\n"
#define REQNORIGHTS "HTTP/1.1 403 Forbidden\r\n"\
				"Date: %s\r\n"\
				"Server: myhttpd/1.0.0 (Ubuntu64)\r\n"\
				"Content-Length: %ld\r\n"\
				"Content-Type: text/html\r\n"\
				"Connection: Closed\r\n"\
				"\r\n"
#define MSG1 "<html>Sorry dude, couldn't find this file.</html>"
#define MSG2 "<html>Trying to access this file but don't think i can make it.</html>"

struct arg_struct
{
	int sock;
	int c_sock;
	char *root_dir;
};

extern fd_set set, readfds;
extern short int shtdwn_flag; 
// must be set to NULL  
extern buflist *buffer;
extern int count;
extern int served_pages , total_bytes;			//for stats 
extern pthread_t *tid;
extern pthread_t prod;
extern pthread_mutex_t mtx , clock_mtx , stat_mtx, shtdw_mtx;
extern pthread_cond_t cond_nonempty;
extern struct timeb start,end;

void* commands(void*);
void* worker(void*);
void* producer(void*);

#endif