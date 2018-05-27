#include "client.h"

void* worker_client(void* args)
{
	char buf[270];
	args_struct *arg = (args_struct*)args;
	// int sockfd = arg->fd;
	int sockfd;
	int port = arg->port;
	char *host = arg->host;
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;; //= arg->serverptr;
	char *cur_url;
	struct hostent *rem;
	// while(1)
	// {

		printf("Thread %ld\n", pthread_self());
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

		//pop head from queue , if queue not empty , take siteX/... and form the msg
		//must free cur_url after i am done
		// if (count > 0)
		// {
		// 	printf("EDWWW\n");
		// 	print_c(&queue);
		// 	// pop_head_c(&queue, &cur_url);
		// }
		// else
		// 	exit(1);
		printf("HRE\n");
		// printf("URL: %s\n", cur_url);
		//construct GET request
		snprintf(buf, sizeof(buf), REQUEST,cur_url,host);
		printf("BUF: %s\n", buf);
		snprintf(buf, sizeof(buf), REQUEST,"/site0/page0_27199.html","/home/thanos/Desktop/root_dir");
		//to steila
		if (write(sockfd,buf,strlen(buf))<0)
			printf("Fail req\n");
		//kleinw to socket gia grapsimo , mporei na xreiazetai mporei kai oxi
		shutdown(sockfd, SHUT_WR);
			
		int data_read=0;
		int total_data=0;
		char *rr=NULL;
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
		rr = strstr(buf,"<!DOCTYPE html>");
		printf("%s\n", rr);
		printf("BUFLEN %ld , rrlen %ld\n", strlen(buf),strlen(rr));
		//takes only header 
		char *new = malloc(sizeof(char)*(strlen(buf)-strlen(rr)+1));
		memcpy(new, buf, strlen(buf)-strlen(rr));
		printf("NEW\n%s--",new);
		//copy of buf , in order no to lose data from strtok
		//use it to take code and length of response from HTTP header
		char *token, delim[] = "\r\n";
		int response_len = -1 , code = -1;
		token = strtok(new, delim);
		while (token != NULL)
		{
			printf("TOKEN. %s\n",token);
			check_response(token,&code,&response_len);
			if (code != -1 && response_len != -1)
				break;
			token = strtok(NULL, delim);
		}
		free(new);
		//apo header thelw na dw code kai length , code gia na dw an tha to dextw kai length gia malloc
		//Newbuf-> pairnei ta extra poy phre o buf prin kai ta upoloipa apo th selida
		//kai ta apothikeyei sto arxeio poy prepei na ftiaxtei
		char *Newbuf = malloc(sizeof(char)*(response_len+1));
		memcpy(Newbuf, rr, strlen(rr));

		total_data = strlen(rr);
		while ((data_read = read(sockfd,&Newbuf[total_data],(response_len+1)-total_data)) > 0)
			total_data += data_read;
		printf("TOTAL_DATA %d \n", total_data);
		printf("-------------------------\n");
		printf("%s\n", Newbuf);
		printf("-------------------------\n");

		printf("Code %d , len %d\n", code,response_len);

		//pairnw apo Newbuf ola ta links kai ta vazw sthn oura
		const char *needle = "<a href=";
		char *p = Newbuf, *tmpp;
		char link[100];
		int i;
		while ( (p=strstr(p,needle)) != NULL ) 
   		{
			printf("#.%s\n",p);
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
	        push_c(&queue, link);
	    	printf("\n%s\n", link);
	    	//reset buffer, ready for next link
	    	memset(link, 0, sizeof(link));
	    	i=0;
	        // total++; //total occurences of string searched
    	}
		FILE *fp = fopen("/home/thanos/Desktop/save_dir/file.html", "w");
		//grapse sto arxeio eite th vrhke th selida eite oxi
		if (code > 0)
		{
			//pws ftiaxnw ta arxeia ? kai pws mpainoun sto swsto directory
			fwrite(Newbuf, 1, strlen(Newbuf), fp);
			//close file
		}

		close(sockfd); /* Close socket and exit */
			
	// }
}