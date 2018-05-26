#include "check_response.h"

int check_response(char *buffer,int *code,int *response_len)
{
	char *tmpbuf = malloc(sizeof(char)*(strlen(buffer)+1));
	memset(tmpbuf, 0, strlen(buffer)+1);
	memcpy(tmpbuf, buffer, strlen(buffer));
	tmpbuf[strlen(tmpbuf)] = '\0';

	char *token, delim[] = " \0", *rest = tmpbuf;
	token = strtok_r(rest,delim,&rest);
	if (!strcmp(token,"HTTP/1.1"))
	{
		token = strtok_r(rest,delim,&rest);
		*code = atoi(token);
		// return 1;
	}
	else if (!strcmp(token, "Content-Length:"))
	{
		token = strtok_r(rest,delim,&rest);
		*response_len = atoi(token);
	}


	free(tmpbuf);
	return 1;
}