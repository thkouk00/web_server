#include "valid_request.h"

int valid_request(char *buffer,int loop,char **target)
{
	if (*target != NULL)
		return 1;
	int word = 0;
	char *token;
	// char *target;
	token = strtok(buffer," \0");
	word++;
	if (loop == 1)
	{
		if (strcmp(token,"GET"))
			return -1;
		token = strtok(NULL," \0");
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
			token = strtok(NULL," \0");
			word++;
		}
		if (word == 4)
			return 1;
		else
			return -1;
	}
	
	if (strcmp(token, "Host:"))
		return -1;
	token = strtok(NULL," \0");
	word++;
	while (token != NULL)
	{
		if (word == 2)
		{
			//here target represents host
			*target = malloc(sizeof(char)*(strlen(token)+1));
			memcpy(*target, token, strlen(token));
		}
		token = strtok(NULL," \0");
		word++;
	}
	if (word == 3)
		return 1;
	else
		return -1;
}