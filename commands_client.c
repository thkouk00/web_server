#include "client.h"

void* commands_client(void* nsock)
{
	int *command_sock = nsock;
	char buf[256];
	memset(buf, 0, sizeof(buf));
	printf("Command port printing\n");
	//response through socket not stdout
	while (read(*command_sock, buf, 256)>0)
	{	
		printf("COMMAND\n");
		buf[strlen(buf)-1] = '\0';
		if(strlen(buf)==0)
			break;
		if (!strcmp(buf, "STATS"))
		{
			pthread_mutex_lock(&clock_mtx);
			pthread_mutex_lock(&stat_mtx);
			//mporei na xreiazetai lock kai to stat_mtx , check it
			ftime(&end);				//stop timer
			int hours;
			memset(buf, 0, sizeof(buf));
			if ((end.time-start.time)/60 >= 60)
			{
				hours = ((end.time-start.time)/60);
				sprintf(buf, "Crawler up for %02d:%02d:%02d.%02d, downloaded %d pages, %d bytes\n",hours/60,(hours/60)%60,hours%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
				// printf("Server up for %02d:%02d:%02d.%02d, served %d pages, %d bytes\n",hours/60,(hours/60)%60,hours%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
			}
			else
				sprintf(buf, "Crawler up for %02ld:%02ld.%02d, downloaded %d pages, %d bytes\n",(end.time-start.time)/60,(end.time-start.time)%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
				// printf("Server up for %02ld:%02ld.%02d, served %d pages, %d bytes\n",(end.time-start.time)/60,(end.time-start.time)%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
			write(*command_sock, buf, strlen(buf));
			pthread_mutex_unlock(&stat_mtx);
			pthread_mutex_unlock(&clock_mtx);
		}
		else if (!strcmp(buf, "SEARCH"))
		{
			char *search_buffer;
			printf("SEARCH asked\n");
			if (working_threads > 0)
			{
				write(*command_sock, "Crawling in progress, please try again later.", strlen("Crawling in progress, please try again later."));
				continue;
			}
			int pid = fork();
			if (pid == 0)
			{
				FILE *fp = fopen("/home/thanos/Desktop/input_dirs", "w");
				DIR *dir;
				struct dirent *entr;
				// printf("SACEDIR->%s.\n", save_dir);
				dir = opendir(save_dir);
				if (dir == NULL)
				{
					printf("error opendir\n");
					exit(1);
				}
				while ((entr = readdir(dir)) != NULL)
				{
					if (!strcmp(entr->d_name, "..") || !strcmp(entr->d_name, "."))
						continue;
					search_buffer = malloc(sizeof(char)*(strlen(save_dir)+strlen(entr->d_name)+2));
					memset(search_buffer, 0, strlen(save_dir)+strlen(entr->d_name)+1);
					memcpy(search_buffer, save_dir, strlen(save_dir));
					strcat(search_buffer, "/");
					strcat(search_buffer, entr->d_name);
					search_buffer[strlen(search_buffer)] = '\0';
					fwrite(search_buffer, 1, strlen(search_buffer), fp);
					fwrite("\n", 1, strlen("\n"), fp);
					printf("%s\n", entr->d_name);
					free(search_buffer);
				}
				fclose(fp);
				closedir(dir);
				dup2(*command_sock, 0);
				dup2(*command_sock, 1);
				execl("/home/thanos/di/syspro/jobExecutor/jobExecutor","jobExecutor" ,"-d","/home/thanos/Desktop/input_dirs","-w","2",NULL);
				printf("ERROR exec\n");
			}
			else
			{
				waitpid(pid,NULL,0);
			}
		}
		else if (!strcmp(buf, "SHUTDOWN"))
		{
			printf("Shutting down Crawler\n");
			shtdwn_flag = 1;
			close(*command_sock);
			break;
		}

		printf("Commandport received : %s\n", buf);
		memset(buf, 0, 256);
	}
	printf("Epistrefw main\n");
	// write(*command_sock,"Response from server",strlen("Response from server"));
	return (void*)1;
}