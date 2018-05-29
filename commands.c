#include "server.h"

void* child2(void* nsock) 
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
				sprintf(buf, "Server up for %02d:%02d:%02d.%02d, served %d pages, %d bytes\n",hours/60,(hours/60)%60,hours%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
				// printf("Server up for %02d:%02d:%02d.%02d, served %d pages, %d bytes\n",hours/60,(hours/60)%60,hours%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
			}
			else
				sprintf(buf, "Server up for %02ld:%02ld.%02d, served %d pages, %d bytes\n",(end.time-start.time)/60,(end.time-start.time)%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
				// printf("Server up for %02ld:%02ld.%02d, served %d pages, %d bytes\n",(end.time-start.time)/60,(end.time-start.time)%60,(1000+(end.millitm-start.millitm)%1000)/10,served_pages,total_bytes);
			write(*command_sock, buf, strlen(buf));
			pthread_mutex_unlock(&stat_mtx);
			pthread_mutex_unlock(&clock_mtx);
		}
		if (!strcmp(buf, "SHUTDOWN"))
		{
			printf("Shutting down server\n");
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