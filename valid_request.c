#include "valid_request.h"

int valid_request(char *buffer,int loop,char **target)
{
	printf("FUNC %s\n",buffer);
	int word = 0;
	char *token;
	// char *target;
	token = strtok(buffer," \0");
	if (loop == 1)
	{
		word++;
		if (strcmp(token,"GET"))
		{
			printf("GET error\n");
			return -1;
		}
		while (token != NULL)
		{
			token = strtok(NULL," \0");
			word++;
			if (word == 2)
			{	
				printf("MPAIKA\n");
				*target = malloc(sizeof(char)*(strlen(token)+1));
				// target = token;
				memcpy(*target,token,strlen(token));
			}
			else if (word == 3)
			{
				if (strcmp(token, "HTTP/1.1"))
				{
					printf("HTTP error\n");
					return -1;
				}
			}
		}
		if (word == 4)
			return 1;
		else
			return -1;
	}
	if (strcmp(token, "Host:"))
		return -1;
	while (token != NULL)
	{
		token = strtok(NULL," \0");
		word++;
		//here target represents host
		*target = malloc(sizeof(char)*(strlen(token)+1));
		memcpy(*target, token, strlen(token));
	}
	if (word == 3)
		return 1;
	else
		return -1;
}