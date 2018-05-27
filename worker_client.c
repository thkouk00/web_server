#include "client.h"

void* worker_client(void* args)
{
	char buf[270];
	args_struct *arg = (args_struct*)args;
	int sockfd = arg->fd;
	struct sockaddr *serverptr = arg->serverptr;
	while(1)
	{
		printf("Thread %ld\n", pthread_self());
		if (connect(sockfd, serverptr, sizeof(server)) == -1)
			perror("Failed to connect");

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

		FILE *fp = fopen("/home/thanos/Desktop/save_dir/file.html", "w");
		//grapse sto arxeio eite th vrhke th selida eite oxi
		if (code > 0)
		{
			//pws ftiaxnw ta arxeia ? kai pws mpainoun sto swsto directory
			fwrite(Newbuf, 1, strlen(Newbuf), fp);
			//close file
		}

		close(sockfd); /* Close socket and exit */
			
	}
}