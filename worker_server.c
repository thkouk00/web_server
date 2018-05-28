#include "server.h"

void* worker(void* arg)
{
	struct arg_struct *arg_strct = (void*)arg;
	int sock = arg_strct->sock;					//den to xrisimopoiw kapou
	int fd;
	int loop, valid, invalid, res;
	char requestbuff[1000]; 			// NAME_MAX + 13 for standard chars (GET , HTTP etc) + 1 for \0
	char *target = NULL;
	char *host = NULL;
	char *path = NULL;
	char date[32];
	char responsebuf[400];
	char fileresponse;
	char *root_dir = arg_strct->root_dir;
	
	while(1)
	{
		pthread_mutex_lock(&mtx);
		while (count == 0 && !shtdwn_flag)
		{
			// printf("WOKE UP %ld\n",pthread_self());
			printf("Prin wait flag is %d and thread %ld\n", shtdwn_flag,pthread_self());
			pthread_cond_wait(&cond_nonempty, &mtx);
		}
		if (shtdwn_flag)
		{
			// shutdown(fd, SHUT_RD);
			// pthread_mutex_lock(&shtdw_mtx);
			// printf("MPIKAKAKAKA worker\n");
			// pthread_cond_broadcast(&cond_nonempty);
			// pthread_mutex_unlock(&shtdw_mtx);
			printf("Worker %ld in\n", pthread_self());
			pthread_mutex_unlock(&mtx);
			// pthread_exit((void*)1);
			break;
		}
		// take first fd from queue
		pop_head(&buffer, &fd);
		count--;
		printf("Evgala fd %d , count %d\n", fd,count);
		printf("Worker %ld\n", pthread_self());
		pthread_mutex_unlock(&mtx);
		valid = invalid = 0;
		loop = 1;
		// reset buffer
		memset(requestbuff, 0, sizeof(requestbuff));
		int data_read=0;
		int total_data=0;
		while ((data_read = read(fd, &requestbuff[total_data], (sizeof(requestbuff)-total_data)))>0)
		{	
			total_data += data_read;
		}
		printf("%s", requestbuff);
		//must end with blank line
		if (strncmp(&requestbuff[strlen(requestbuff)-4],"\r\n\r\n", 4))
		{
			printf("No blank line at end\n");
			invalid = 1;
		}
		else
		{
			// requestbuff[strlen(requestbuff)] = '\0';
			char *token , delim[]="\r\n";
			char *tmp;
			token = strtok(requestbuff, delim);
			while (token!=NULL)
			{
				printf("AA %s.\n", token);
				tmp = malloc(sizeof(char)*(strlen(token)+1));
				memset(tmp, 0, strlen(token)+1);
				memcpy(tmp, token, strlen(token));
				tmp[strlen(tmp)] = '\0';
				// // token[strlen(token)] = '\0';
				// if (loop == 1)
				// if (!strncmp(token, "GET ",4))
				if (!strncmp(tmp, "GET ", 4))	
					valid = valid_request(tmp, loop, &target);
				else
					valid = valid_request(tmp, loop, &host);
				// {
				// 	printf("STELNW %s.\n", token);
				// 	printf("#.%s.\n", requestbuff);
				// }
				if (valid < 0)
				{
					printf("INVALID request\n");
					invalid = 1;
					//***to evala meta apo valgrind na to dokimasw,mporei na skasei***
					free(tmp);
					break;
				}
				free(tmp);
				loop++;
				token = strtok(NULL, delim);
				printf("BACK %s.\n",token);
				// printf("%s\n", requestbuff);
			}
			printf("TARGET %s,HOST %s\n", target,host);
		}

		//form time for http response
		time_t now =time(0);
		struct tm tm = *gmtime(&now);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
		if (!invalid)
		{
			// path = malloc(sizeof(char)*(strlen(target)+strlen(host)));
			//evala +1
			target[strlen(target)] = '\0';
			path = malloc(sizeof(char)*(strlen(target)+strlen(root_dir)+1));
			memset(path, 0, (strlen(target)+strlen(root_dir)+1));
			sprintf(path,"%s%s",root_dir,target);
			path[strlen(path)] = '\0';
			printf("PATH is %s\n", path);
			// printf("Path is %s\n", path);
			//check if file exists and if we have rights to read it
			if ((res = access(path, F_OK)) == 0)
			{
				printf("OK file exists\n");
				if ((res = access(path, R_OK)) < 0)
				{	
					//send 403 msg
					snprintf(responsebuf, sizeof(responsebuf), REQNORIGHTS,date,strlen(MSG2));
					// send response to client
					write(fd, responsebuf, strlen(responsebuf));
					write(fd, MSG2, strlen(MSG2));
				}
				else
				{
					FILE *fp = fopen(path, "r");
					int fsize;
					ssize_t bytes_written;
					ssize_t total_bytes_written = 0;
					fseek(fp, 0, SEEK_END);
					fsize = ftell(fp);
					//send 200 msg
					snprintf(responsebuf, sizeof(responsebuf), REQFOUND,date,fsize);
					fseek(fp, 0, SEEK_SET);
					// send response to client
					write(fd, responsebuf, strlen(responsebuf));
					char *tmpbuf = malloc(sizeof(char)*fsize);
					// kai ayto htan extra
					memset(tmpbuf, 0, fsize);
					fread(tmpbuf,fsize,1,fp);
					while (total_bytes_written != fsize)
					{
						bytes_written = write(fd,&tmpbuf[total_bytes_written],fsize - total_bytes_written);
						if (bytes_written < 0)
							continue;
						total_bytes_written += bytes_written;	
						printf("total_bytes_written %ld\n", total_bytes_written);
					}
					pthread_mutex_lock(&stat_mtx);
					served_pages++;
					total_bytes += fsize;
					pthread_mutex_unlock(&stat_mtx);
					free(tmpbuf);
					fclose(fp);
				}
			}
			else
			{
				//send 404 msg
				snprintf(responsebuf, sizeof(responsebuf), REQNOTFOUND,date,strlen(MSG1));
				// send response to client
				write(fd, responsebuf, strlen(responsebuf));
				write(fd, MSG1, strlen(MSG1));
			}
			
			// free path
			free(path);
			path = NULL;
		}
		//reset variables for next fd
		if (target)
		{
			free(target);
			target = NULL;
		}
		if (host)
		{
			free(host);
			host = NULL;
		}
		// close connection
		// close(fd);

		//check if needed lock
		
		close(fd);
		// if (shtdwn_flag)
		// {
		// 	// shutdown(fd, SHUT_RD);
		// 	pthread_mutex_lock(&shtdw_mtx);
		// 	printf("MPIKAKAKAKA worker\n");
		// 	pthread_cond_broadcast(&cond_nonempty);
		// 	pthread_mutex_unlock(&shtdw_mtx);
		// 	// pthread_exit((void*)1);
		// 	break;
		// }
	}
	printf("---EXW APO WHILE\n");
	pthread_exit((void*)1);
	// return (void*)1;
}