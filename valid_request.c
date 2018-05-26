#include "valid_request.h"
#define REQ1 "GET %s HTTP/1.1"
#define REQ2 "Host: %s"
#define REQ3 "User-Agent:" 

int valid_request(char *buffer,int loop,char **target)
{
	char *tt = malloc(sizeof(char)*(strlen(buffer)+1));
	memset(tt, 0, strlen(buffer)+1);
	memcpy(tt, buffer, strlen(buffer));
	tt[strlen(tt)] = '\0';
	printf("PHRA %s\n", tt);
	char *rest = tt;
	// if (!strncmp(&buffer[strlen(buffer)-2], "\r\n",2))
	// if (buffer[strlen(buffer)-1]=='\n')
	// 	printf("nai einai\n");
	if (*target != NULL)
		return 1;
	int word = 0;
	char *token;
	char delim[]=" \0";
	// char *target;
	token = strtok_r(rest,delim,&rest);
	printf("TOKEN %s\n", token);
	word++;
	if (loop == 1)
	{
		if (strcmp(token,"GET"))
			return -1;
		token = strtok_r(rest,delim,&rest);
		printf("TOKEN1 %s\n", token);
		word++;
		while (token != NULL)
		{
			// token = strtok(NULL," \0");
			// word++;
			if (word == 2)
			{	
				*target = malloc(sizeof(char)*(strlen(token)+1));
				memcpy(*target,token,strlen(token));
			}
			else if (word == 3)
			{
				if (strcmp(token, "HTTP/1.1"))
					return -1;
			}
			token = strtok_r(rest,delim,&rest);
			printf("TOKEN2 %s\n", token);
			word++;
		}
		printf("HERE\n");
		if (word == 4)
			return 1;
		else
			return -1;
	}
	
	if (strcmp(token, "Host:"))
		return -1;
	token = strtok_r(rest,delim,&rest);
	word++;
	while (token != NULL)
	{
		if (word == 2)
		{
			//here target represents host
			*target = malloc(sizeof(char)*(strlen(token)+1));
			memcpy(*target, token, strlen(token));
		}
		token = strtok_r(rest,delim,&rest);
		word++;
	}
	if (word == 3)
		return 1;
	else
		return -1;

}

