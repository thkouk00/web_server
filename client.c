#include "client.h"

#define REQUEST "GET %s HTTP/1.1\r\n"\
				"Host: %s\r\n"\
				"\r\n"

void* worker_client();
void perror_exit(char *message);
void Usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s -h host_or_IP -p port -c command_port -t num_of_threads -d save_dir starting_URL\n", prog_name);
}

fd_set set, readfds;
short int shtdwn_flag, exit_flag;
char *save_dir;
// must be set to NULL  
url_queue *queue = NULL;
url_queue *checked_urls = NULL;
int count, working_threads = 0;
int served_pages = 0, total_bytes = 0;			//for stats 
pthread_t *tid;
pthread_mutex_t mtx , clock_mtx , stat_mtx;
pthread_cond_t cond_nonempty;
struct timeb start,end;

int main(int argc, char* argv[])
{
	char *host_or_IP, *starting_URL;
	int port, command_port, nthr, args_taken=0;

	if (argc != 12)
	{
		Usage(argv[0]);
		exit(1);
	}
	else
	{
		for (int i=0;i<argc;i++)
		{
			if (!strcmp(argv[i], "-h"))
			{
				host_or_IP = malloc(sizeof(char)*(strlen(argv[i+1])+1));
				memset(host_or_IP, 0, strlen(argv[i+1])+1);
				memcpy(host_or_IP, argv[i+1], strlen(argv[i+1]));
				args_taken++;
			}
			else if (!strcmp(argv[i], "-p"))
			{
				port = atoi(argv[i+1]);
				args_taken++;
			}
			else if (!strcmp(argv[i], "-c"))
			{
				command_port = atoi(argv[i+1]);
				args_taken++;
			}
			else if (!strcmp(argv[i], "-t"))
			{
				nthr = atoi(argv[i+1]);
				args_taken++;
			}
			else if (!strcmp(argv[i], "-d"))
			{
				save_dir = malloc(sizeof(char)*(strlen(argv[i+1])+1));
				memset(save_dir, 0, strlen(argv[i+1])+1);
				memcpy(save_dir, argv[i+1], strlen(argv[i+1]));
				// save_dir[strlen(save_dir)] = '\0';
			}
			else if (i == argc-1)
			{
				starting_URL = malloc(sizeof(char)*(strlen(argv[i])+1));
				memset(starting_URL, 0, strlen(argv[i])+1);
				memcpy(starting_URL, argv[i], strlen(argv[i]));
				// starting_URL[strlen(starting_URL)] = '\0';
				args_taken++;
			}
		}
		if (args_taken != 5)
		{
			Usage(argv[0]);
			exit(1);
		}

	}
	// printf("Host:%s\n", host_or_IP);
	// printf("URL %s\n", starting_URL);
	// printf("Dir %s\n", save_dir);
	
	int i, sock, c_sock, sockopt_val = 1, command_sock;
	socklen_t serverlen;
	char buf[270];
	//used to connect to command_port
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	
	pthread_t *tid; 
	DIR* dir = opendir(save_dir);
	struct dirent *de;
	//check if dir exists , if not create directory
	if (dir)
	{
		//check code
		while ((de = readdir(dir)) != NULL)
		{
			if (!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))
				continue;	
			// printf("%s\n", de->d_name);
			
		}
	}
	else	// rwx for user and group , rx for others
		mkdir(save_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	//SETTING UP CONNECTION TO COMMAND PORT
	
	//create socket
	if ((c_sock=socket(AF_INET,SOCK_STREAM,0)) == -1)
		perror("Failed to create socket :: client");	

	//enable reuse option for socket
	if (setsockopt(c_sock, SOL_SOCKET, SO_REUSEADDR, &sockopt_val, sizeof(int)) == -1)
	{
		perror("Failed: setsockopt");
		exit(1);
	}

	server.sin_family = AF_INET;					//internet domain
	server.sin_addr.s_addr = htonl(INADDR_ANY);		//any ip address
	server.sin_port = htons(command_port);	

	if (bind(c_sock,serverptr,sizeof(server)) == -1)
		perror("Failed to bind socket to command port");

	if (listen(c_sock,5) == -1)
		perror("Failed: listen");

	//END OF SETUP FOR CONNECTION

	//initialize mutexes and cond_var
	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&clock_mtx, 0);
	pthread_mutex_init(&stat_mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);
	
	//insert url in queue
	push_c(&queue, &checked_urls, starting_URL, NULL);
	count = 1;
	if (nthr == 0)
		nthr = 2;
	tid = malloc(sizeof(pthread_t)*nthr);
	// arguments to sent to worker threads
	args_struct arg;
	arg.fd = sock;
	arg.port = port;
	arg.host = host_or_IP;
	// arg.serverptr = serverptr;
	//thread_pool
	for (i=0;i<nthr;i++)
		pthread_create(tid+i, 0, worker_client, (void*)&arg);
	ftime(&start);			//start timer
	
	if (!exit_flag)
	{
		while (1)
		{
			serverlen = sizeof(server);
			if ((command_sock = accept(c_sock, serverptr, &serverlen)) == -1)
				perror("Failed: accept for command port");

			commands_client(&command_sock);
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
	
	//free memory
	free(tid);
	free(host_or_IP);
	free(save_dir);
	free(starting_URL);
	freelist_c(&queue);
	freelist_c(&checked_urls);
	
	// close(sock); /* Close socket and exit */
}

void perror_exit(char *message)
{
	perror(message);
	exit(EXIT_FAILURE);
}
