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
#include <sys/timeb.h>
#include <sys/sendfile.h>	//more efficient way to send file than read-write
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


void Usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s -p serving_port -c command_port -t num_of_threads -d root_dir\n", prog_name);
}

// **NOTICE**
// if server tries to write to a closed socket by client  
// SIGPIPE raised so i have to handle it

// pass arguments for thread in pthread_create
struct arg_struct
{
	int arg1;
	int arg2;
};

void* child2(void* nsock);

void* worker(void* arg);
void* producer(void* args);

void sigchld_handler(int sig);


fd_set set, readfds;
short int shtdwn_flag = 0;
// must be set to NULL  
buflist *buffer = NULL;
int count=0;
int served_pages = 0 , total_bytes = 0;			//for stats 
pthread_t *tid;
pthread_t prod;
pthread_mutex_t mtx , clock_mtx , stat_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
// time_t start = -1 , end = -1;
// time_t start,end;
struct timeb start,end;
// struct timeval t1,t2;

int main(int argc , char* argv[])
{
	int i , sockopt_val=1;
	int nthr, port, command_port, sock, c_sock, newsock, command_sock;
	char *root_dir;
	socklen_t cmdlen;
	struct sockaddr_in server , cmd;
	struct sockaddr *serverptr = (struct sockaddr*)&server ;
	struct sockaddr *cmdptr= (struct sockaddr*)&cmd;
	struct hostent *rem;

	//check if all arguments exist
	if (argc != 9)
	{
		Usage(argv[0]);
		exit(1);
	}
	else
	{
		//take arguments
		for (i=0;i<argc;i++)
		{
			if (!strcmp("-p",argv[i]))
				port = atoi(argv[i+1]);
			else if (!strcmp("-c",argv[i]))
				command_port = atoi(argv[i+1]);
			else if (!strcmp("-t",argv[i]))
			{
				nthr = atoi(argv[i+1]);
				if (nthr == 0)
					nthr = 2;		//default value
			}
			else if (!strcmp("-d",argv[i]))
			{
				root_dir = malloc(sizeof(char)*(strlen(argv[i+1])+1));
				memcpy(root_dir, argv[i+1], strlen(argv[i+1]));
			}
		}
	}

	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&clock_mtx, 0);
	pthread_mutex_init(&stat_mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);
	

	//create socket for clients
	if ((sock=socket(AF_INET,SOCK_STREAM,0)) == -1)
		perror("Failed to create socket");

	//create socket in order to receive commands from command line 
	if ((c_sock=socket(AF_INET,SOCK_STREAM,0)) == -1)
		perror("Failed to create socket");	

	// enable option for both sockets 
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockopt_val, sizeof(int)) == -1)
	{
		perror("Failed: setsockopt");
		exit(1);
	}

	if (setsockopt(c_sock, SOL_SOCKET, SO_REUSEADDR, &sockopt_val, sizeof(int)) == -1)
	{
		perror("Failed: setsockopt");
		exit(1);
	}

	server.sin_family = AF_INET;					//internet domain
	server.sin_addr.s_addr = htonl(INADDR_ANY);		//any ip address
	server.sin_port = htons(port);					//given port
	//bind socket to address
	if (bind(sock,serverptr,sizeof(server)) == -1)
		perror("Failed to bind socket to port");

	server.sin_port = htons(command_port);	
	if (bind(c_sock,serverptr,sizeof(server)) == -1)
		perror("Failed to bind socket to port");	
	
	//listen for connections
	if (listen(sock,128) == -1)
		perror("Failed: listen");

	if (listen(c_sock,1) == -1)
		perror("Failed: listen");
	printf("Listening for connections to port %d and commands from port %d\n", port,command_port);
	
	// struct for passing arguments
	//useless i think , check it later
	struct arg_struct arg_strct;
	arg_strct.arg1 = sock;
	arg_strct.arg2 = c_sock;

	// thread pool 
	tid = malloc(sizeof(pthread_t)*nthr);
	//create #nthr threads
	for (int i=0;i<nthr;i++)
		pthread_create(tid+i, 0, worker, (void*)&sock);
	//create one thread for inserting fd to buff
	pthread_create(&prod, 0, producer, (void*)&arg_strct);
	ftime(&start);			//start timer

	// wait for connection via netcat
	// to take commands
	// close server when "SHUTDOWN" arrive from cmd
	while (1)
	{	
		cmdlen = sizeof(cmd);
		if ((command_sock = accept(c_sock, cmdptr, &cmdlen)) == -1)
			perror("Failed: accept for command port");

		child2(&command_sock);
		if (shtdwn_flag)
		{
			shutdown(command_sock, SHUT_RDWR);
			break;
		}
	}	
	printf("JERE\n");

	for (int i=0;i<nthr;i++)
		pthread_join(tid[i], NULL);
	printf("EDW\n");
	pthread_join(prod, NULL);
	printf("EDW2\n");
	//free queue
	freelist(&buffer);
	//free
	free(root_dir);
	free(tid);
}


void* worker(void* arg)
{
	int *sock = arg;
	int fd;
	int loop, valid, invalid, res;
	char requestbuff[1000]; 			// NAME_MAX + 13 for standard chars (GET , HTTP etc) + 1 for \0
	char *target = NULL;
	char *host = NULL;
	char *path = NULL;
	char date[32];
	char responsebuf[400];
	char fileresponse;
	
	while(1)
	{
		pthread_mutex_lock(&mtx);
		while (count == 0)
			pthread_cond_wait(&cond_nonempty, &mtx);
		// take first fd from queue
		pop_head(&buffer, &fd);
		count--;
		printf("Evgala fd %d , count %d\n", fd,count);
		printf("Worker %ld\n", pthread_self());
		pthread_mutex_unlock(&mtx);
		valid = invalid = 0;
		loop = 1;
		// reset buffer
		memset(requestbuff, 0, sizeof(requestbuff));
		int data_read=0;
		int total_data=0;
		while ((data_read = read(fd, &requestbuff[total_data], (sizeof(requestbuff)-total_data)))>0)
		{	
			total_data += data_read;
		}
		printf("%s", requestbuff);
		//must end with blank line
		if (strncmp(&requestbuff[strlen(requestbuff)-4],"\r\n\r\n", 4))
		{
			printf("No blank line at end\n");
			invalid = 1;
		}
		else
		{
			// requestbuff[strlen(requestbuff)] = '\0';
			char *token , delim[]="\r\n";
			char *tmp;
			token = strtok(requestbuff, delim);
			while (token!=NULL)
			{
				printf("AA %s.\n", token);
				tmp = malloc(sizeof(char)*(strlen(token)+1));
				memset(tmp, 0, strlen(token)+1);
				memcpy(tmp, token, strlen(token));
				tmp[strlen(tmp)] = '\0';
				// // token[strlen(token)] = '\0';
				// if (loop == 1)
				// if (!strncmp(token, "GET ",4))
				if (!strncmp(tmp, "GET ", 4))	
					valid = valid_request(tmp, loop, &target);
				else
					valid = valid_request(tmp, loop, &host);
				// {
				// 	printf("STELNW %s.\n", token);
				// 	printf("#.%s.\n", requestbuff);
				// }
				if (valid < 0)
				{
					printf("INVALID request\n");
					invalid = 1;
					break;
				}
				free(tmp);
				loop++;
				token = strtok(NULL, delim);
				printf("BACK %s.\n",token);
				// printf("%s\n", requestbuff);
			}
			printf("TARGET %s,HOST %s\n", target,host);
		}

		//form time for http response
		time_t now =time(0);
		struct tm tm = *gmtime(&now);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
		if (!invalid)
		{
			path = malloc(sizeof(char)*(strlen(target)+strlen(host)));
			sprintf(path, "%s%s",host,target);
			// printf("Path is %s\n", path);
			//check if file exists and if we have rights to read it
			if ((res = access(path, F_OK)) == 0)
			{
				printf("OK file exists\n");
				if ((res = access(path, R_OK)) < 0)
				{	
					//send 403 msg
					snprintf(responsebuf, sizeof(responsebuf), REQNORIGHTS,date,strlen(MSG2));
					// send response to client
					write(fd, responsebuf, strlen(responsebuf));
					write(fd, MSG2, strlen(MSG2));
				}
				else
				{
					FILE *fp = fopen(path, "r");
					int fsize;
					ssize_t bytes_written;
					ssize_t total_bytes_written = 0;
					fseek(fp, 0, SEEK_END);
					fsize = ftell(fp);
					//send 200 msg
					snprintf(responsebuf, sizeof(responsebuf), REQFOUND,date,fsize);
					fseek(fp, 0, SEEK_SET);
					// send response to client
					write(fd, responsebuf, strlen(responsebuf));
					char *tmpbuf = malloc(sizeof(char)*fsize);
					fread(tmpbuf,fsize,1,fp);
					while (total_bytes_written != fsize)
					{
						bytes_written = write(fd,&tmpbuf[total_bytes_written],fsize - total_bytes_written);
						if (bytes_written < 0)
							continue;
						total_bytes_written += bytes_written;
						
					}
					pthread_mutex_lock(&stat_mtx);
					served_pages++;
					total_bytes += fsize;
					pthread_mutex_unlock(&stat_mtx);
					free(tmpbuf);
				}
			}
			else
			{
				//send 404 msg
				snprintf(responsebuf, sizeof(responsebuf), REQNOTFOUND,date,strlen(MSG1));
				// send response to client
				write(fd, responsebuf, strlen(responsebuf));
				write(fd, MSG1, strlen(MSG1));
			}
			
			// free path
			free(path);
		}
		//reset variables for next fd
		if (target)
		{
			free(target);
			target = NULL;
		}
		if (host)
		{
			free(host);
			host = NULL;
		}
		// close connection
		// close(fd);

		//check if needed lock
		if (shtdwn_flag)
		{
			shutdown(fd, SHUT_RDWR);
			pthread_exit((void*)1);
		}
		else
			close(fd);
	}
}

void* producer(void* args)
{
	struct arg_struct* arg = args;
	int sock = arg->arg1;
	int c_sock = arg->arg2;
	int newsock;
	int highfd, res;
	socklen_t clientlen, cmdlen;
	struct sockaddr_in client, cmd;
	struct sockaddr *clientptr= (struct sockaddr*)&client ;
	struct sockaddr *cmdptr= (struct sockaddr*)&cmd;

	FD_ZERO(&set);
	FD_SET(sock, &set);
	highfd = sock;
	while(1)
	{
		readfds = set;
		res = select(highfd+1, &readfds, NULL, NULL, NULL);
		if (res < 0)
			printf("ERROR SELECT\n");
		else if (res > 0)
		{
			pthread_mutex_lock(&mtx);
			if ((newsock = accept(sock, clientptr, &clientlen)) == -1)
				perror("Failed: accept");

			// insert fd to buffer
			push(&buffer,newsock);
			count++;
			printf("New insertion %d , count %d\n", newsock,count);
			printf("Eimai %ld\n", pthread_self());
			pthread_cond_broadcast(&cond_nonempty);
			pthread_mutex_unlock(&mtx);
		}
		if (shtdwn_flag)
			pthread_exit((void*)1);
	}
}





void* child2(void* nsock) 
{
	int *command_sock = nsock;
	char buf[256];
	printf("Command port printing\n");
	while (read(*command_sock, buf, 256)>0)
	{	
		buf[strlen(buf)-1] = '\0';
		if(strlen(buf)==0)
			break;
		if (!strcmp(buf, "STATS"))
		{
			pthread_mutex_lock(&clock_mtx);
			//mporei na xreiazetai lock kai to stat_mtx , check it
			ftime(&end);				//stop timer
			int hours;
			if ((end.time-start.time)/60 >= 60)
			{
				hours = ((end.time-start.time)/60);
				printf("Server up for %02d:%02d:%02d.%02d, served %d pages, %d bytes\n",hours/60,(hours/60)%60,hours%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
			}
			else
				printf("Server up for %02ld:%02ld.%02d, served %d pages, %d bytes\n",(end.time-start.time)/60,(end.time-start.time)%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
			pthread_mutex_unlock(&clock_mtx);
		}
		if (!strcmp(buf, "SHUTDOWN"))
		{
			printf("Shutting down server\n");
			shtdwn_flag = 1;
			// close(*command_sock);
			break;
		}

		printf("Commandport received : %s\n", buf);
		memset(buf, 0, 256);
	}

	// write(*command_sock,"Response from server",strlen("Response from server"));
	return (void*)1;
}


void sigchld_handler(int sig) 
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}
