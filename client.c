#include <stdio.h>
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <sys/stat.h>	/* mkdir */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h> /* read, write, close */
#include <netdb.h> /* gethostbyaddr */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <dirent.h>
#include "check_response.h"


#define REQUEST "GET %s HTTP/1.1\r\n"\
				"Host: %s\r\n"\
				"\r\n"
#define RESPONSE "HTTP/1.1 200 OK\r\n"\
				"Date: XXX, XX XXX XXXX XX:XX:XX GMT"\
				"Server: myhttpd/1.0.0 (Ubuntu64)"\
				"Content-Length: 65081"\
				"Content-Type: text/html"\
				"Connection: Closed"

void perror_exit(char *message);
void Usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s -h host_or_IP -p port -c command_port -t num_of_threads -d save_dir starting_URL\n", prog_name);
}


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
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;
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
	// char *filename = 
	FILE *fp = fopen("/home/thanos/Desktop/save_dir/file.html", "w");

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		perror("Failed to create socket");
	if ((rem = gethostbyname(host_or_IP)) == NULL)
	{	
		herror("gethostbyname"); 
		exit(1);
	}
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	server.sin_port = htons(port);
	printf("SS %s\n", rem->h_addr);
	if (connect(sock, serverptr, sizeof(server)) == -1)
		perror("Failed to connect");
	printf("Connecting to %s port %d\n", host_or_IP, port);
	
	
		
	snprintf(buf, sizeof(buf), REQUEST,"/site0/page0_27199.html","/home/thanos/Desktop/root_dir");
		// buf[strlen(buf)] = '\0';
	if (write(sock,buf,strlen(buf))<0)
		printf("Fail req\n");
	shutdown(sock, SHUT_WR);
		
	int data_read=0;
	int total_data=0;
	char *rr=NULL;
	memset(buf, 0, sizeof(buf));
	while ((data_read = read(sock,&buf[total_data],sizeof(buf)-total_data)) > 0)
	{
		total_data += data_read;
		printf("MPIKA %d\n", total_data);
		// if ((rr = strstr(buf, "\r\n\r\n"))!=NULL)
		// 	break; 
	} 
	// rr = strstr(buf, "\r\n\r\n");
	rr = strstr(buf,"<!DOCTYPE html>");
	printf("%s\n", rr);
	printf("BUFLEN %ld , rrlen %ld\n", strlen(buf),strlen(rr));
	//takes only header
	char *new = malloc(sizeof(char)*(strlen(buf)-strlen(rr)+1));
	memcpy(new, buf, strlen(buf)-strlen(rr));
	printf("NEW\n%s--",new);
	//twra tha parw ta ypoloipa , an exei parei kai na parw me write + ta ypoloipa
	char *Newbuf = malloc(sizeof(char)*650802);
	memcpy(Newbuf, rr, strlen(rr));

	total_data = strlen(rr);
	while ((data_read = read(sock,&Newbuf[total_data],65802-total_data)) > 0)
		total_data += data_read;
	printf("TOTAL_DATA %d and %ld\n", total_data,sizeof(Newbuf)-total_data);
	printf("-------------------------\n");
	printf("%s\n", Newbuf);
	printf("-------------------------\n");
	printf("%s\n", buf);
	char *token, delim[] = "\r\n";
	int response_len = -1 , code = -1, body_flag = 0;
	//copy of buf , in order no to lose data from strtok
	//use it to take code and length of response from HTTP header
	char *tempbuf = malloc(sizeof(char)*(strlen(buf)+1)); 
	memset(tempbuf, 0, strlen(buf)+1);
	memcpy(tempbuf, buf, strlen(buf));
	token = strtok(tempbuf, delim);
	while (token != NULL)
	{
		printf("TOKEN. %s\n",token);
		check_response(token,&code,&response_len);
		if (code != -1 && response_len != -1)
			break;
		token = strtok(NULL, delim);
	}
	free(tempbuf);
	//take body of HTTP response
	char *body;
	body = strstr(buf, "<!DOCTYPE html>");
	printf("%s\n", body);	
	printf("length %ld\n", strlen(body));
	printf("Code %d , len %d\n", code,response_len);
	fwrite(Newbuf, 1, strlen(Newbuf), fp);

	close(sock); /* Close socket and exit */
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