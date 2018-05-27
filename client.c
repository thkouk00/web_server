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
// must be set to NULL  
url_queue *queue;
int count;
int served_pages, total_bytes;			//for stats 
pthread_t *tid;
pthread_mutex_t mtx , clock_mtx , stat_mtx;
pthread_cond_t cond_nonempty;
struct timeb start,end;
struct sockaddr_in server;

int main(int argc, char* argv[])
{
	char *host_or_IP, *save_dir, *starting_URL;
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
				memcpy(save_dir, argv[i+1], strlen(argv[i+1]));
			}
			else if (i == argc-1)
			{
				starting_URL = malloc(sizeof(char)*(strlen(argv[i])+1));
				memcpy(starting_URL, argv[i], strlen(argv[i]));
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
	url_queue *queue = NULL;
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

	// if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	// 	perror("Failed to create socket");
	// if ((rem = gethostbyname(host_or_IP)) == NULL)
	// {	
	// 	herror("gethostbyname"); 
	// 	exit(1);
	// }
	// server.sin_family = AF_INET;
	// memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	// server.sin_port = htons(port);

	//insert url in queue
	push_c(&queue, starting_URL);
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
	arg.serverptr = serverptr;
	//edw tha prepei na mpoun ta threads , connect/thread , 1 mono fdsock
	for (i=0;i<nthr;i++)
		pthread_create(tid+i, 0, worker_client, (void*)&arg);
	// printf("EDWWWW\n");
	// while(1);
	// if (connect(sock, serverptr, sizeof(server)) == -1)
	// 	perror("Failed to connect");
	// printf("Connecting to %s port %d\n", host_or_IP, port);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//worker_client.c
	// FILE *fp = fopen("/home/thanos/Desktop/save_dir/file.html", "w");
	//pairnw ta aitimata apo thn oura logika , tha to dw auto
	//ftiaxnw to minima
	// snprintf(buf, sizeof(buf), REQUEST,"/site0/page0_27199.html",host_or_IP);
	// // //to steila
	// if (write(sock,buf,strlen(buf))<0)
	// 	printf("Fail req\n");
	// // //kleinw to socket gia grapsimo , mporei na xreiazetai mporei kai oxi
	// shutdown(sock, SHUT_WR);
		
	// int data_read=0;
	// int total_data=0;
	// char *rr=NULL;
	// memset(buf, 0, sizeof(buf));
	// // //diabazei header + kati akoma
	// while ((data_read = read(sock,&buf[total_data],sizeof(buf)-total_data)) > 0)
	// {
	// 	total_data += data_read;
	// 	printf("MPIKA %d\n", total_data);
	// 	// if ((rr = strstr(buf, "\r\n\r\n"))!=NULL)
	// 	// 	break; 
	// } 
	// // //brisko pou xekina to body tou minimatos
	// rr = strstr(buf,"<!DOCTYPE html>");
	// printf("%s\n", rr);
	// printf("BUFLEN %ld , rrlen %ld\n", strlen(buf),strlen(rr));
	// //takes only header 
	// char *new = malloc(sizeof(char)*(strlen(buf)-strlen(rr)+1));
	// memcpy(new, buf, strlen(buf)-strlen(rr));
	// printf("NEW\n%s--",new);
	// //copy of buf , in order no to lose data from strtok
	// //use it to take code and length of response from HTTP header
	// char *token, delim[] = "\r\n";
	// int response_len = -1 , code = -1;
	// token = strtok(new, delim);
	// while (token != NULL)
	// {
	// 	printf("TOKEN. %s\n",token);
	// 	check_response(token,&code,&response_len);
	// 	if (code != -1 && response_len != -1)
	// 		break;
	// 	token = strtok(NULL, delim);
	// }
	// free(new);
	// //apo header thelw na dw code kai length , code gia na dw an tha to dextw kai length gia malloc
	// //Newbuf-> pairnei ta extra poy phre o buf prin kai ta upoloipa apo th selida
	// //kai ta apothikeyei sto arxeio poy prepei na ftiaxtei
	// char *Newbuf = malloc(sizeof(char)*(response_len+1));
	// memcpy(Newbuf, rr, strlen(rr));

	// total_data = strlen(rr);
	// while ((data_read = read(sock,&Newbuf[total_data],(response_len+1)-total_data)) > 0)
	// 	total_data += data_read;
	// printf("TOTAL_DATA %d \n", total_data);
	// printf("-------------------------\n");
	// printf("%s\n", Newbuf);
	// printf("-------------------------\n");

	// printf("Code %d , len %d\n", code,response_len);

	// //grapse sto arxeio eite th vrhke th selida eite oxi
	// // if (code > 0)
	// // {
	// // 	//pws ftiaxnw ta arxeia ? kai pws mpainoun sto swsto directory
	// // 	fwrite(Newbuf, 1, strlen(Newbuf), fp);
	// // }
	for (int i=0;i<nthr;i++)
		pthread_join(tid[i], NULL);
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
