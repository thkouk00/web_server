#include <stdio.h>
#include <sys/types.h> /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h> /* read, write, close */
#include <netdb.h> /* gethostbyaddr */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */

#define REQUEST "GET %s HTTP/1.1\r\n"\
				"Host: %s\r\n"\
				"\r\n"

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
	char buf[1000];
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;

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
	
	//loopa client
	// while(1)
	// {
	// for (int i =0;i<3;i++)
	// {
	// 	fgets(buf,sizeof(buf),stdin);
	// 	if (write(sock,buf,strlen(buf))<0)
	// 		printf("Fail req1\n");
	// 	memset(buf, 0, sizeof(buf));
	// }
		
		snprintf(buf, sizeof(buf), REQUEST,"/site0/page0_21.html","/home/thanos/Desktop/root_dir");
		// buf[strlen(buf)] = '\0';
		if (write(sock,buf,strlen(buf))<0)
			printf("Fail req\n");
		shutdown(sock, SHUT_WR);
		// // sleep(3);
		// sleep(3);
		// if (write(sock,REQ3,strlen(REQ3))<0)
		// 	printf("Fail req3\n");
		// int read_size = read(sock,buf,sizeof(buf));
		int data_read=0;
		int total_data=0;
		memset(buf, 0, sizeof(buf));
		while ((data_read = read(sock,&buf[total_data],sizeof(buf)-total_data))>0)
		{
			total_data += data_read;
		} 
		//evgala auto gia dokimi , doyleuei kai xwris ayto
		// buf[strlen(buf)-1] = '\0';
		printf("%s\n", buf);
		// if (read_size<0)
		// 	printf("Error receive\n");
		// else
		// {
		// 	printf("read_size %d\n", read_size);
		// 	printf("%s\n", buf);
		// }
		printf("ALALAL\n");
		// while ((read_size=read(sock,buf,sizeof(buf)))>=0)
		// {
		// 	if (read_size == 0)
		// 		continue;
		// 	printf("%s\n", buf);
		// 	memset(buf, 0, sizeof(buf));
			
		// }
	// }
	// while (read(sock, buf, 39)<0);
	// printf("Buf:\n");
	// printf("%s", buf);


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