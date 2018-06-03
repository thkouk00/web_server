#include "client.h"

void* worker_client(void* args)
{
	char buf[270];
	args_struct *arg = (args_struct*)args;
	// int sockfd = arg->fd;
	int sockfd;
	int port = arg->port;
	int queue_count = 0;
	char *host = arg->host;
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	char *cur_url;
	struct hostent *rem;
	
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			perror("Failed to create socket");
		if ((rem = gethostbyname(host)) == NULL)
		{	
			herror("gethostbyname"); 
			exit(1);
		}
		server.sin_family = AF_INET;
		memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
		server.sin_port = htons(port);

		if (connect(sockfd, serverptr, sizeof(server)) == -1)
			perror("Failed to connect :: worker_client");

		pthread_mutex_lock(&mtx);
		
		// printf("Thread %ld\n", pthread_self());
		//wait until queue has link to extract
		while (urls_left(&queue) == 0)
		{
			//if queue is empty and all workers are waiting exit
			if (working_threads == 0)
			{
				pthread_cond_broadcast(&cond_nonempty);
				pthread_mutex_unlock(&mtx);
				// shutdown(sockfd, SHUT_RDWR);
				// pthread_exit((void *)1);
				exit_flag = 1;
				break;
			}
			pthread_cond_wait(&cond_nonempty, &mtx);
		}
		if (exit_flag)
		{
			close(sockfd);
			// pthread_mutex_unlock(&mtx);
			break;
		}
		
		working_threads++;
		//pop head from queue 
		pop_head_c(&queue, &cur_url);
		printf("POP %s\n", cur_url);
		// print_c(&queue);
		count--;
		//must free cur_url after i am done
		pthread_mutex_unlock(&mtx);
		//construct GET request
		snprintf(buf, sizeof(buf), REQUEST,cur_url,host);
		buf[strlen(buf)] = '\0';
		//send GET request
		if (write(sockfd,buf,strlen(buf))<0)
			printf("Fail req\n");
		//close write end for socket
		shutdown(sockfd, SHUT_WR);
			
		int data_read=0;
		int total_data=0;
		// char *start_of_body=NULL;
		memset(buf, 0, sizeof(buf));
		//diabazei header + kati akoma
		while ((data_read = read(sockfd,&buf[total_data],sizeof(buf)-total_data)) > 0)
		{
			total_data += data_read;
		} 
		//brisko pou xekina to body tou minimatos
		char *start_of_body = strstr(buf,"<!DOCTYPE html>");
		if (start_of_body == NULL)
		{
			pthread_mutex_lock(&mtx);
			working_threads--;
			pthread_mutex_unlock(&mtx);
			continue;
		}
		//for valgrind error
		start_of_body[strlen(start_of_body)] = '\0';
		
		//takes only header 
		char *header = malloc(sizeof(char)*(strlen(buf)-strlen(start_of_body)+1));
		memset(header, 0, strlen(buf)-strlen(start_of_body)+1);
		memcpy(header, buf, strlen(buf)-strlen(start_of_body));
		//copy of buf , in order not to lose data from strtok
		//use it to take code and length of response from HTTP header
		char *token, delim[] = "\r\n";
		int response_len = -1 , code = -1;
		token = strtok(header, delim);
		while (token != NULL)
		{
			check_response(token,&code,&response_len);
			if (code != -1 && response_len != -1)
				break;
			token = strtok(NULL, delim);
		}
		free(header);
		//check for code returned (200, 403,404)
		//if code != 200 then go to next html file
		if (code != 200)
			continue;

		//apo header thelw na dw code kai length , code gia na dw an tha to dextw kai length gia malloc
		//Newbuf-> pairnei ta extra poy phre o buf prin kai ta upoloipa apo th selida
		//kai ta apothikeyei sto arxeio poy prepei na ftiaxtei
		char *body = malloc(sizeof(char)*(response_len+1));
		memset(body, 0, response_len+1);
		memcpy(body, start_of_body, strlen(start_of_body));
		//extra to vala valgr
		total_data = 0;
		total_data = strlen(start_of_body);
		while ((data_read = read(sockfd,&body[total_data],(response_len+1)-total_data)) > 0)
			total_data += data_read;
		// printf("TOTAL_DATA %d \n", total_data);
		// printf("-------------------------\n");
		// printf("Code %d , len %d\n", code,response_len);
		shutdown(sockfd, SHUT_RD);
		//extract links and puth them to queue
		const char needle[] = "<a href=";
		char *p = body, *tmpp=NULL;
		char link[150];
		int i = 0;
		memset(link, 0, sizeof(link));
		while ( (p=strstr(p,needle)) != NULL ) 
   		{
	        p += strlen(needle);
	        tmpp = p;
	        while (*tmpp != '>')
	        {	
	        	link[i] = *tmpp;
	        	i++;
	        	tmpp++;
	        }
	        link[strlen(link)] = '\0';
	        //edw to link einai etoimo , push it in queue
	        pthread_mutex_lock(&mtx);
	       	push_c(&queue, &checked_urls, link, cur_url);
	       	printf("PUSH %s\n", link);
	       	printf("Left %d\n", urls_left(&queue));
	        //**no need for this i think , check it later**
	        // count++;
	        // if (urls_left(&queue)==0)
	        	pthread_cond_broadcast(&cond_nonempty);
	        pthread_mutex_unlock(&mtx);
	    	//reset buffer, ready for next link
	    	memset(link, 0, sizeof(link));
	    	i=0;
    	}
 		char *path_to_file = malloc(sizeof(char)*(strlen(save_dir)+strlen(cur_url)+1));
 		memset(path_to_file, 0, strlen(save_dir)+strlen(cur_url)+1);
 		memcpy(path_to_file, save_dir, strlen(save_dir));
 		strcat(path_to_file, cur_url);
 		
		char dir[50];
		memset(dir, 0, sizeof(dir));
		char *temp = cur_url;
		int dir_char_count = 0;
		i=0;
		while (dir_char_count != 2 && *temp != '\0')
		{
			if (*temp == '/')
				dir_char_count++;
			dir[i] = *temp;
			i++;
			temp++; 
		}
		dir[strlen(dir)] = '\0';
		char *dir_path = malloc(sizeof(char)*(strlen(save_dir)+strlen(dir)+1));
		memset(dir_path, 0, strlen(save_dir)+strlen(dir)+1);
		memcpy(dir_path, save_dir, strlen(save_dir));
		strcat(dir_path, dir);
		//give dir read/write/search permissions for owner and group
		//and with read/search permissions for others
		mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		FILE *fp = fopen(path_to_file, "w");
		if (fp == NULL)
			perror("fopen Error\n");
		// //grapse sto arxeio eite th vrhke th selida eite oxi
		// printf("Path %s.\n", path_to_file);
		if (code > 0)
			fwrite(body, 1, strlen(body), fp);
		// close file
		fclose(fp);
		//server closes sockfd
		//extra 3/6
		close(sockfd); /* Close socket and exit */
		free(body);
		free(dir_path);
		free(path_to_file);
		pthread_mutex_lock(&mtx);
		working_threads--;
		pthread_mutex_unlock(&mtx);
		pthread_mutex_lock(&stat_mtx);
		served_pages++;
		total_bytes += response_len;
		pthread_mutex_unlock(&stat_mtx);
		
	}
	return (void*)1;
}