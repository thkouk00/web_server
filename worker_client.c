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
	
	// bzero((char*)&server,sizeof(server));
	memset(buf, 0, sizeof(buf));
	while(1)
	{
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
			perror("Failed to connect");
		printf("Thread %ld\n", pthread_self());

		pthread_mutex_lock(&mtx);
		queue_count = urls_left(&queue);
		printf("QUEUE COUNT %d\n", queue_count);
		//crawling is over
		if (working_threads == 0 && queue_count == 0)
		{
			pthread_cond_broadcast(&cond_nonempty);
			pthread_mutex_unlock(&mtx);
			break;
		}
		//wait until queue has link to extract
		while (count == 0)
			pthread_cond_wait(&cond_nonempty, &mtx);
		working_threads++;
		//pop head from queue 
		pop_head_c(&queue, &cur_url);
		printf("HRERE %s\n",cur_url);
		print_c(&queue);
		count--;
		//must free cur_url after i am done
		pthread_mutex_unlock(&mtx);
		//construct GET request
		snprintf(buf, sizeof(buf), REQUEST,cur_url,host);

		printf("BUF: %s\n", buf);
		buf[strlen(buf)] = '\0';
		//send GET request
		if (write(sockfd,buf,strlen(buf))<0)
			printf("Fail req\n");
		//close write for socket
		shutdown(sockfd, SHUT_WR);
			
		int data_read=0;
		int total_data=0;
		char *start_of_body=NULL;
		memset(buf, 0, sizeof(buf));
		//diabazei header + kati akoma
		while ((data_read = read(sockfd,&buf[total_data],sizeof(buf)-total_data)) > 0)
		{
			total_data += data_read;
			printf("MPIKA %d\n", total_data);
			// if ((rr = strstr(buf, "\r\n\r\n"))!=NULL)
			// 	break; 
		} 
		//brisko pou xekina to body tou minimatos
		start_of_body = strstr(buf,"<!DOCTYPE html>");
		printf("VGHKA\n");
		if (start_of_body == NULL)
			printf("EINAI NULL\n");
		printf("%s\n", start_of_body);
		printf("BUFLEN %ld , rrlen %ld\n", strlen(buf),strlen(start_of_body));
		//takes only header 
		char *header = malloc(sizeof(char)*(strlen(buf)-strlen(start_of_body)+1));
		memset(header, 0, strlen(buf)-strlen(start_of_body)+1);
		memcpy(header, buf, strlen(buf)-strlen(start_of_body));
		printf("HEADER\n%s--",header);
		//copy of buf , in order not to lose data from strtok
		//use it to take code and length of response from HTTP header
		char *token, delim[] = "\r\n";
		int response_len = -1 , code = -1;
		token = strtok(header, delim);
		while (token != NULL)
		{
			printf("TOKEN. %s\n",token);
			check_response(token,&code,&response_len);
			if (code != -1 && response_len != -1)
				break;
			token = strtok(NULL, delim);
		}
		free(header);
		//apo header thelw na dw code kai length , code gia na dw an tha to dextw kai length gia malloc
		//Newbuf-> pairnei ta extra poy phre o buf prin kai ta upoloipa apo th selida
		//kai ta apothikeyei sto arxeio poy prepei na ftiaxtei
		char *body = malloc(sizeof(char)*(response_len+1));
		memset(body, 0, response_len+1);
		memcpy(body, start_of_body, strlen(start_of_body));

		total_data = strlen(start_of_body);
		while ((data_read = read(sockfd,&body[total_data],(response_len+1)-total_data)) > 0)
			total_data += data_read;
		printf("TOTAL_DATA %d \n", total_data);
		printf("-------------------------\n");
		// body[strlen(body)] = '\0';
		printf("%s\n", body);
		printf("-------------------------\n");

		printf("Code %d , len %d\n", code,response_len);

		//pairnw apo Newbuf ola ta links kai ta vazw sthn oura
		//htan const kai *
		const char needle[] = "<a href=";
		char *p = body, *tmpp;
		char link[150];
		int i = 0;
		memset(link, 0, sizeof(link));
		printf("***************************\n");
		printf("BODY:: %s\n", p);
		printf("***************************\n");
		while ( (p=strstr(p,needle)) != NULL ) 
   		{
			printf("#.%s\n",p);
	        p += strlen(needle);
	        printf("##.%s\n",p);
	        tmpp = p;
	        printf("&&EINAI %c\n",*tmpp);
	        while (*tmpp != '>')
	        {	
	        	link[i] = *tmpp;
	        	printf("###. %c\n",link[i]);
	        	i++;
	        	tmpp++;
	        }
	        link[strlen(link)] = '\0';
	        //edw to link einai etoimo , push it in queue
	    	printf("TELEIOSA TO LINK\n%s\n",link);
	        pthread_mutex_lock(&mtx);
	    	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	    	// if (search_c(&queue, link) == 1)
	    	// {
	    	// 	printf("SEARCH -1\n");
	    	// 	// memset(link, 0, sizeof(link));
	    	// 	i = 0;
	    	// 	pthread_cond_broadcast(&cond_nonempty);
	    	// 	pthread_mutex_unlock(&mtx);
	    	// 	continue;
	    	// }
	       	push_c(&queue, &checked_urls, link, cur_url);
	        // push_c(&checked_urls, link,cur_url);
	        print_c(&queue);
	        printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	        count++;
	        pthread_cond_broadcast(&cond_nonempty);
	        pthread_mutex_unlock(&mtx);
	    	//reset buffer, ready for next link
	    	memset(link, 0, sizeof(link));
	    	i=0;
	        // total++; //total occurences of string searched
    	}
 		char *path_to_file = malloc(sizeof(char)*(strlen(save_dir)+strlen(cur_url)+1));
 		memset(path_to_file, 0, strlen(save_dir)+strlen(cur_url)+1);
 		memcpy(path_to_file, save_dir, strlen(save_dir));
 		strcat(path_to_file, cur_url);
 		path_to_file[strlen(path_to_file)] = '\0';
		// FILE *fp = fopen(path_to_file, "w");
		// if (fp == NULL)
		// 	perror("fopen Error\n");
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
		//grapse sto arxeio eite th vrhke th selida eite oxi
		printf("Path %s\n", path_to_file);
 		printf("FTANW EDW\nBODY^:\n");
		// printf("%s\n^BODY\n", body);
		if (code > 0)
			fwrite(body, 1, strlen(body), fp);
		// close file
		fclose(fp);
		//server closes sockfd
		// close(sockfd); /* Close socket and exit */
		free(body);
		pthread_mutex_lock(&mtx);
		working_threads--;
		pthread_mutex_unlock(&mtx);
		
	}
	return (void*)1;
}