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
short int shtdwn_flag;
char *save_dir;
// must be set to NULL  
url_queue *queue = NULL;
url_queue *checked_urls = NULL;
int count, working_threads = 0;
int served_pages, total_bytes;			//for stats 
pthread_t *tid;
pthread_mutex_t mtx , clock_mtx , stat_mtx;
pthread_cond_t cond_nonempty;
struct timeb start,end;
struct sockaddr_in server;

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
	printf("Host:%s\n", host_or_IP);
	printf("URL %s\n", starting_URL);
	printf("Dir %s\n", save_dir);
	
	int i, sock;
	char buf[270];
	// url_queue *queue = NULL;
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	// struct hostent *rem;
	pthread_t *tid; 
	DIR* dir = opendir(save_dir);
	struct dirent *de;
	//check if dir exists , if not create directory
	if (!dir)
		mkdir(save_dir,0760);
	//ftiaxnw meta na kanei purge
	while ((de = readdir(dir)) != NULL)
	{
		if (!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))
			continue;	
		printf("%s\n", de->d_name);
		// if (rmdir(de->d_name)<0)
		// 	printf("ERROR\n");
	}


	//initialize mutexes and cond_var
	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&clock_mtx, 0);
	pthread_mutex_init(&stat_mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);
	// pthread_mutex_init(&shtdw_mtx, 0);
	
	//insert url in queue
	push_c(&queue, &checked_urls, starting_URL, NULL);
	// push_c(&checked_urls, starting_URL,NULL);
	count = 1;
	print_c(&queue);
	printf("PERSASASAS\n");
	if (nthr == 0)
		nthr = 2;
	tid = malloc(sizeof(pthread_t)*nthr);
	// arguments to sent to worker threads
	args_struct arg;
	arg.fd = sock;
	arg.port = port;
	arg.host = host_or_IP;
	// arg.serverptr = serverptr;
	//edw tha prepei na mpoun ta threads , connect/thread , 1 mono fdsock
	for (i=0;i<nthr;i++)
		pthread_create(tid+i, 0, worker_client, (void*)&arg);
	//thread gia command line
	while (1)
	{
		if (working_threads == 0 && (urls_left(&queue)) == 0)
		{
			printf("EPITELOYS MPIKA EDW\n");
			pthread_cond_broadcast(&cond_nonempty);
			pthread_mutex_unlock(&mtx);
			break;
		}
	}

	for (int i=0;i<nthr;i++)
		pthread_join(tid[i], NULL);
	printf("EFTASA EDW\n");
	// close(sock); /* Close socket and exit */
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//free memory
	free(host_or_IP);
	free(save_dir);
	free(starting_URL);
	
}

void perror_exit(char *message)
{
	perror(message);
	exit(EXIT_FAILURE);
}
