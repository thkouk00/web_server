#include "server.h"

void* worker(void* arg)
{
	struct arg_struct *arg_strct = (void*)arg;
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
		// while (count == 0 && !shtdwn_flag)
		while(nodes_left(&buffer) <= 0 && !shtdwn_flag)
		{
			printf("Ready to wait %ld\n",pthread_self());
			pthread_cond_wait(&cond_nonempty, &mtx);
			printf("XIPNISA %d apo %ld\n", count,pthread_self());
		}
		printf("BGHKA APO WAIT me stoixeia %d and %ld\n", nodes_left(&buffer),pthread_self());
		if (shtdwn_flag)
		{
			pthread_mutex_unlock(&mtx);
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
		printf("PRIN MPO READ %ld\n",pthread_self());
		while ((data_read = read(fd, &requestbuff[total_data], (sizeof(requestbuff)-total_data)))>0)
		{	
			total_data += data_read;
			if (!strncmp(&requestbuff[strlen(requestbuff)-4],"\r\n\r\n", 4))
				break;
			printf("phra %s\n", requestbuff);
			printf("HERE %d\n", total_data);
		}
		shutdown(fd, SHUT_RD);
		printf("%s", requestbuff);
		//must end with blank line
		if (strncmp(&requestbuff[strlen(requestbuff)-4],"\r\n\r\n", 4))
		{
			// printf("No blank line at end\n");
			invalid = 1;
		}
		else
		{
			char *token , delim[]="\r\n";
			char *tmp;
			token = strtok(requestbuff, delim);
			while (token!=NULL)
			{
				tmp = malloc(sizeof(char)*(strlen(token)+1));
				memset(tmp, 0, strlen(token)+1);
				memcpy(tmp, token, strlen(token));
				tmp[strlen(tmp)] = '\0';
				if (!strncmp(tmp, "GET ", 4))	
					valid = valid_request(tmp, loop, &target);
				else
					valid = valid_request(tmp, loop, &host);
				
				if (valid < 0)
				{
					// printf("INVALID request\n");
					invalid = 1;
					// free(tmp);
					break;
				}
				free(tmp);
				loop++;
				token = strtok(NULL, delim);
				printf("TOKEN LOOP\n");
			}
		}

		//form time for http response
		time_t now =time(0);
		struct tm tm = *gmtime(&now);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
		if (!invalid)
		{
			target[strlen(target)] = '\0';
			path = malloc(sizeof(char)*(strlen(target)+strlen(root_dir)+1));
			memset(path, 0, (strlen(target)+strlen(root_dir)+1));
			sprintf(path,"%s%s",root_dir,target);
			path[strlen(path)] = '\0';
			//check if file exists and if we have rights to read it
			if ((res = access(path, F_OK)) == 0)
			{
				// printf("OK file exists\n");
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
					memset(tmpbuf, 0, fsize);
					fread(tmpbuf,fsize,1,fp);
					while (total_bytes_written != fsize)
					{
						bytes_written = write(fd,&tmpbuf[total_bytes_written],fsize - total_bytes_written);
						if (bytes_written < 0)
							continue;
						total_bytes_written += bytes_written;	
						// printf("total_bytes_written %ld\n", total_bytes_written);
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
		//to vala extra
		// shutdown(fd, SHUT_WR);
		close(fd);
	}
	//na dw an apla kanw return kai oxi pthread_exit
	pthread_exit((void*)1);
	// return (void*)1;
}