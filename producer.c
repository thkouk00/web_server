#include "server.h"

void* producer(void* args)
{
	struct arg_struct* arg = args;
	int sock = arg->sock;
	int c_sock = arg->c_sock;
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
		if (shtdwn_flag)
		{
			pthread_exit((void*)1);
			return (void*)1;
		}
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
		printf("FLAG %d\n", shtdwn_flag);
		// if (shtdwn_flag)
		// {
		// 	pthread_exit((void*)1);
		// 	return (void*)1;
		// }
	}
}