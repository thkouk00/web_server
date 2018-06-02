#include "server.h"

void* producer(void* args)
{
	struct arg_struct* arg = args;
	int sock = arg->sock;
	int c_sock = arg->c_sock;
	int newsock;
	int highfd, res;
	struct sockaddr_in client = {0}, cmd;
	struct sockaddr *clientptr= (struct sockaddr*)&client ;
	struct sockaddr *cmdptr= (struct sockaddr*)&cmd;
	//initialize value 
	socklen_t clientlen = sizeof(struct sockaddr_in), cmdlen;

	FD_ZERO(&set);
	FD_SET(sock, &set);
	highfd = sock;
	while(1)
	{
		
		if ((newsock = accept(sock, clientptr, &clientlen)) == -1)
		{	
			if (shtdwn_flag)
			{
				pthread_mutex_unlock(&mtx);
				pthread_exit((void*)1);
				// return (void*)1;
			}
			else
				perror("Failed: accept");
		}
		pthread_mutex_lock(&mtx);

		// insert fd to buffer
		push(&buffer,newsock);
		count++;
		// printf("New insertion %d , count %d\n", newsock,count);
		// printf("From thread %ld\n", pthread_self());
		pthread_cond_broadcast(&cond_nonempty);
		pthread_mutex_unlock(&mtx);
		
		// if (shtdwn_flag)
		// {
		// 	pthread_exit((void*)1);
		// 	return (void*)1;
		// }
	}
}