#include "valid_request.h"
#define REQ1 "GET %s HTTP/1.1"
#define REQ2 "Host: %s"
#define REQ3 "User-Agent:" 

int valid_request(char *buffer,int loop,char **target)
{
	//na to kanw free kiolas auto
	char *tt = malloc(sizeof(char)*(strlen(buffer)+1));
	memset(tt, 0, strlen(buffer)+1);
	memcpy(tt, buffer, strlen(buffer));
	tt[strlen(tt)] = '\0';
	printf("PHRA %s\n", tt);
	char *rest = tt;

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
		{
			free(tt);
			return -1;
		}
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
				//twra to vala valgrind
				memset(*target, 0, strlen(token)+1);
				memcpy(*target,token,strlen(token));
			}
			else if (word == 3)
			{
				if (strcmp(token, "HTTP/1.1"))
				{
					free(tt);
					return -1;
				}
			}
			token = strtok_r(rest,delim,&rest);
			printf("TOKEN2 %s\n", token);
			word++;
		}
		printf("HERE\n");
		free(tt);
		if (word == 4)
			return 1;
		else
			return -1;
	}
	
	if (strcmp(token, "Host:"))
	{
		free(tt);
		return -1;
	}
	token = strtok_r(rest,delim,&rest);
	word++;
	while (token != NULL)
	{
		if (word == 2)
		{
			//here target represents host
			*target = malloc(sizeof(char)*(strlen(token)+1));
			//twra to vala valgrind
			memset(*target, 0, strlen(token)+1);
			memcpy(*target, token, strlen(token));
			// *target[strlen(*target)] = '\0';
		}
		token = strtok_r(rest,delim,&rest);
		word++;
	}
	free(tt);
	if (word == 3)
		return 1;
	else
		return -1;

}

