#include "server.h"

void Usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s -p serving_port -c command_port -t num_of_threads -d root_dir\n", prog_name);
}

fd_set set, readfds;
short int shtdwn_flag = 0;
// must be set to NULL  
buflist *buffer = NULL;
int count=0;
int served_pages = 0 , total_bytes = 0;			//for stats 
pthread_t *tid;
pthread_t prod;
pthread_mutex_t mtx , clock_mtx , stat_mtx, shtdw_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
struct timeb start,end;

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
				memset(root_dir, 0, strlen(argv[i+1])+1);
				memcpy(root_dir, argv[i+1], strlen(argv[i+1]));
			}
		}
	}
	
	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&clock_mtx, 0);
	pthread_mutex_init(&stat_mtx, 0);
	pthread_mutex_init(&shtdw_mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);
	
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

	if (listen(c_sock,5) == -1)
		perror("Failed: listen");
	// printf("Listening for connections to port %d and commands from port %d\n", port,command_port);
	
	// struct for passing arguments
	//useless i think , check it later
	struct arg_struct arg_strct;
	arg_strct.sock = sock;
	arg_strct.c_sock = c_sock;
	arg_strct.root_dir = root_dir;

	// thread pool 
	tid = malloc(sizeof(pthread_t)*nthr);
	//create #nthr threads
	for (int i=0;i<nthr;i++)
		pthread_create(tid+i, 0, worker, (void*)&arg_strct);
	//create one thread in order to insert fd to buff
	pthread_create(&prod, 0, producer, (void*)&arg_strct);
	ftime(&start);			//start timer
	
	// wait for connection via netcat
	// to take commands
	// close server when "SHUTDOWN" arrive from cmd
	if (!shtdwn_flag)
	{
		while (1)
		{	
			cmdlen = sizeof(cmd);
			if ((command_sock = accept(c_sock, cmdptr, &cmdlen)) == -1)
				perror("Failed: accept for command port");

			child2(&command_sock);
			if (shtdwn_flag)
			{
				pthread_cond_broadcast(&cond_nonempty);
				// shutdown(command_sock, SHUT_RDWR);
				//wake up select
				shutdown(sock, SHUT_RD);
				break;
			}
		}
	}	

	for (int i=0;i<nthr;i++)
		pthread_join(tid[i], NULL);
	pthread_join(prod, NULL);

	// if (pthread_mutex_destroy(&mtx)<0)
	// 	printf("MUTEX < 0\n");;
	pthread_mutex_destroy(&clock_mtx);
	pthread_mutex_destroy(&stat_mtx);
	pthread_mutex_destroy(&shtdw_mtx);
	pthread_cond_destroy(&cond_nonempty);
	//free queue
	freelist(&buffer);
	//free
	free(root_dir);
	free(tid);
}
