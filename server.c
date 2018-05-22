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

// **NOTICE**
// if server tries to write to a closed socket by client  
// SIGPIPE raised so i have to handle it

// void* child_server(void* nsock);
//void* child2(void* nsock); 
struct arg_struct
{
	int arg1;
	int arg2;
};

void* child2(void* nsock);

void* worker(void* arg);
void* producer(void* args);

void perror_exit(char *message);
void sigchld_handler(int sig);


void Usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s -p serving_port -c command_port -t num_of_threads -d root_dir\n", prog_name);
}

fd_set set, readfds;
int connections[4] = {-2};
int pos=0;
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
	// struct sockaddr_in server, client, cmd;
	struct sockaddr_in server , cmd;
	struct sockaddr *serverptr = (struct sockaddr*)&server ;
	// struct sockaddr *clientptr= (struct sockaddr*)&client ;
	struct sockaddr *cmdptr= (struct sockaddr*)&cmd;
	struct hostent *rem;
	// fd_set set, readfds;

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
	

	//create socket
	if ((sock=socket(AF_INET,SOCK_STREAM,0)) == -1)
		perror("Failed to create socket");

	if ((c_sock=socket(AF_INET,SOCK_STREAM,0)) == -1)
		perror("Failed to create socket");	

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
	printf("Listening for connections to port %d\n", port);
	
	FD_ZERO(&set);
	FD_SET(sock, &set);
	FD_SET(c_sock, &set);

	struct arg_struct arg_strct;
	arg_strct.arg1 = sock;
	arg_strct.arg2 = c_sock;

	tid = malloc(sizeof(pthread_t)*nthr);
	//create #nthr threads
	for (int i=0;i<nthr;i++)
		pthread_create(tid+i, 0, producer, (void*)&sock);
	//create one thread for inserting fd to buff
	pthread_create(&prod, 0, worker, (void*)&arg_strct);

	
	while (1)
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
	while(1)
	{
		pthread_mutex_lock(&mtx);
		while (count == 0)
			pthread_cond_wait(&cond_nonempty, &mtx);
		printf("Evgala fd %d apo thesi %d\n", connections[pos-1],pos-1);
		printf("Worker %ld\n", pthread_self());
		connections[pos] = -2;
		pos--;
		count--;
		pthread_cond_broadcast(&cond_nonfull);
		pthread_mutex_unlock(&mtx);
		// if buff not empty
		// 	take fd
		// else
		// 	wait
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

	while(1)
	{
		
		pthread_mutex_lock(&mtx);
		while (count == 4)
			pthread_cond_wait(&cond_nonfull, &mtx);
		if ((newsock = accept(sock, clientptr, &clientlen)) == -1)
			perror("Failed: accept");

		connections[pos] = newsock;
		printf("New insertion %d at pos %d\n", newsock,pos);
		printf("Eimai %ld\n", pthread_self());
		pos++;
		count++;
		pthread_cond_broadcast(&cond_nonempty);
		pthread_mutex_unlock(&mtx);
		sleep(5);
	}
}





void* child2(void* nsock) 
{
	int *command_sock = nsock;
	char buf[1];
	printf("Command port printing\n");
	while(read(*command_sock, buf, 1) > 0) 
	{ /* Receive 1 char */
		putchar(buf[0]); /* Print received char */
	}
}

void* child_server(void* nsock) 
{
	int *newsock = nsock;
	char buf[1];
	while(read(*newsock, buf, 1) > 0) 
	{ /* Receive 1 char */
		putchar(buf[0]); /* Print received char */
		/* Capitalize character */
		buf[0] = toupper(buf[0]);
		/* Reply */
		if (write(*newsock, buf, 1) < 0)
			perror_exit("write");
	}
	printf("Closing connection.\n");
	close(*newsock); /* Close socket */
}

void sigchld_handler(int sig) 
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(char *message) 
{
	perror(message);
	exit(EXIT_FAILURE);
}