#include <stdio.h>
#include <stdlib.h>			//malloc , exit
#include <sys/wait.h>		//sockets
#include <sys/types.h>		//sockets
#include <sys/select.h>		//select()
#include <netinet/in.h>		//htons,htonl,ntohs,ntohl
#include <netdb.h>			//gethostbyaddr, gethostbyname
#include <sys/socket.h>		//all socket functions
#include <unistd.h>			//fork
#include <ctype.h>			//toupper
#include <signal.h>			//signal
#include <pthread.h>		//threads - compile with -pthread at end
#include <string.h>			//strerror , error printing for threads
#include "buflist.h"		//queue to hold fds
#include "valid_request.h"	//check if GET request is valid

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
short int shtdwn = 0;
// must be set to NULL  
buflist *buffer = NULL;
int count=0;
pthread_t *tid;
pthread_t prod;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

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
				nthr = atoi(argv[i+1]);
			else if (!strcmp("-d",argv[i]))
			{
				root_dir = malloc(sizeof(char)*(strlen(argv[i+1])+1));
				memcpy(root_dir, argv[i+1], strlen(argv[i+1]));
			}
		}
	}

	pthread_mutex_init(&mtx, 0);
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

	// wait for connection via netcat
	// to take commands
	// close server when "SHUTDOWN" arrive from cmd
	while (!shtdwn)
	{	
		cmdlen = sizeof(cmd);
		if ((command_sock = accept(c_sock, cmdptr, &cmdlen)) == -1)
			perror("Failed: accept for command port");

		child2(&command_sock);
	}	


	for (int i=0;i<4;i++)
		pthread_join(tid[i], NULL);
	pthread_join(prod, NULL);

	//free
	free(root_dir);
}


void* worker(void* arg)
{
	int *sock = arg;
	int fd;
	int loop = 1 , valid = 0;
	char requestbuff[270]; 			// NAME_MAX + 13 for standard chars (GET , HTTP etc) + 1 for \0
	char *target = NULL;
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
		while (read(fd, requestbuff, 270)>0)
		{	
			requestbuff[strlen(requestbuff)-1] = '\0';
			if(strlen(requestbuff)==0)
				break;
			printf("Server received : %s\n", requestbuff);
			valid = valid_request(requestbuff,loop,&target);
			if (valid > 0)
				printf("Valid request, continue to host\n");
			else
				printf("Invalid request\n");
			printf("Valid %d %s\n", valid,target);
			memset(requestbuff, 0, 270);
			loop++;
			free(target);
			target = NULL;
			// if valid continue
			// else send back error for the request

		}
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
	}
}





void* child2(void* nsock) 
{
	int *command_sock = nsock;
	char buf[256];
	printf("Command port printing\n");
	// while(read(*command_sock, buf, 256) > 0) 
	// { 
	// 	// putchar(buf[0]); /* Print received char */
	// 	printf("Server: %s\n", buf);
	// }
	while (read(*command_sock, buf, 256)>0)
	{	
		buf[strlen(buf)-1] = '\0';
		if(strlen(buf)==0)
			break;
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
