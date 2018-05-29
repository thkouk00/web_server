#include "buflist.h"

//for server
void push(buflist** head, int fd)
{
	// creat head if doesn't exist
	// then insert node at end of list (queue implementation)
	if (*head == NULL)
	{
		*head = (struct buf_struct*)malloc(sizeof(struct buf_struct));
		(*head)->fd = -1;
		(*head)->next = NULL;
	}
	buflist *cur = *head;
	while (cur->next)
		cur = cur->next;
	cur->next = (struct buf_struct*)malloc(sizeof(buflist));
	cur = cur->next;
	cur->fd = fd;
	cur->next = NULL;
}

// pop first node of queue
void pop_head(buflist** head,int *fd)
{
	buflist *cur = *head;
	buflist *tmp = NULL;
	if (cur->next)
	{
		tmp = cur->next;
		cur->next = tmp->next;
		*fd = tmp->fd;
		free(tmp);
	}
}

// free queue by deleting all nodes
void freelist(buflist** head)
{
	if (*head == NULL)
		exit(1);
	buflist *cur = *head;
	buflist *tmp = *head;
	while (cur->next)
	{
		tmp = cur->next;
		cur->next = tmp->next;
		free(tmp);
		if (cur == NULL)
			break;
	}
	free(*head);
	*head = NULL;
}

// print all nodes in queue
void print(buflist** head)
{
	buflist *cur = *head;
	while (cur->next)
	{
		cur = cur->next;
		printf("Fd is %d\n", cur->fd);
	}
}

//for client
void push_c(url_queue** head, url_queue** queue2,char* url, char* cur_url)
{
	int first_push = 0;
	printf("Phra %s\n", url);
	// create head if doesn't exist
	// then insert node at end of list (queue implementation)
	if (*head == NULL)
	{
		*head = (struct client_struct*)malloc(sizeof(struct client_struct));
		(*head)->url = NULL;
		(*head)->next = NULL;
		first_push = 1;
	}
	
	url_queue *cur = *head , *temp;
	while (cur->next)
		cur = cur->next;
	cur->next = (struct client_struct*)malloc(sizeof(struct client_struct));
	temp = cur;
	cur = cur->next;
	//skip ../
	char *new_url = url;
	if (!strncmp(new_url, "../", 3))
	{
		new_url += 2;
		cur->url = malloc(sizeof(char)*(strlen(new_url)+1));
		memset(cur->url, 0, strlen(new_url)+1);
		memcpy(cur->url,new_url,strlen(new_url));
	}
	else		//else concat url with current dir
	{
		if (!first_push)
		{
			int i, char_count = 0;
			char *buf = malloc(sizeof(char)*strlen(cur_url));
			memset(buf, 0, strlen(cur_url));
			i = 0;
			while (char_count != 2 && *cur_url!='\0')
			{
				if (*cur_url == '/')
					char_count++;
				buf[i] = *cur_url;
				i++;
				cur_url++;
			}
			buf[strlen(buf)] = '\0';
			printf("BUF %s\n", buf);
			cur->url = malloc(sizeof(char)*(strlen(buf)+strlen(url)+1));
			memset(cur->url, 0, strlen(buf)+strlen(url)+1);
			memcpy(cur->url,buf,strlen(buf));
			strcat(cur->url, url);
			free(buf);
		}
		else
		{
			cur->url = malloc(sizeof(char)*(strlen(url)+1));
			memset(cur->url, 0, strlen(url)+1);
			memcpy(cur->url,url,strlen(url));
		}
	}
	cur->url[strlen(cur->url)] = '\0';
	cur->next = NULL;
	if (search_c(queue2, cur->url) == 1)
	{
		printf("EINAI HDH MESA TO %s.\n", cur->url);
		free(cur->url);
		cur->url = NULL;
		free(cur);
		temp->next = NULL;
	}
	else
	{
		printf("**MPAINEI** %s.\n", cur->url);
		push_c2(queue2,cur->url);
	}
}

void push_c2(url_queue** head,char* url)
{
	if (*head == NULL)
	{
		*head = (struct client_struct*)malloc(sizeof(struct client_struct));
		(*head)->url = NULL;
		(*head)->next = NULL;
	}
	
	url_queue *cur = *head;
	while (cur->next)
		cur = cur->next;
	cur->next = (struct client_struct*)malloc(sizeof(struct client_struct));
	cur = cur->next;
	cur->url = malloc(sizeof(char)*(strlen(url)+1));
	memset(cur->url, 0, strlen(url)+1);
	memcpy(cur->url, url, strlen(url));
	cur->url[strlen(cur->url)] = '\0';
	cur->next = NULL;
	printf("CHECKED %s.\n", cur->url);
}

// pop first node of queue
void pop_head_c(url_queue** head,char** url)
{
	if (*head == NULL)
		exit(1);
	url_queue *cur = *head;
	url_queue *tmp = NULL;
	if (cur->next)
	{
		tmp = cur->next;
		cur->next = tmp->next;
		*url = tmp->url;
		free(tmp);
		//free url -> client
	}
	printf("EIMAI POP-> %s\n", *url);
	if (*url == NULL)
	{
		printf("PRINTING\n");
		print_c(head);
	}
}

// free queue by deleting all nodes
void freelist_c(url_queue** head)
{
	url_queue *cur = *head;
	url_queue *tmp = *head;
	while (cur->next)
	{
		tmp = cur->next;
		cur->next = tmp->next;
		free(tmp->url);
		free(tmp);
		if (cur == NULL)
			break;
	}
	free(*head);
	*head = NULL;
}

int search_c(url_queue** head, char* url)
{
	if (*head == NULL)
		return -2;
	url_queue *cur = *head;
	while (cur->next)
	{
		cur = cur->next;
		// url found
		if (!strcmp(cur->url,url))
			return 1;
	}
	// url not found
	return -1;
}

// print all nodes in queue
void print_c(url_queue** head)
{
	url_queue *cur = *head;
	if (cur == NULL)
		exit(1);
	while (cur->next)
	{
		cur = cur->next;
		printf("Url is %s.\n", cur->url);
	}
}

int urls_left(url_queue** head)
{
	if (*head == NULL)
		return -1;
	url_queue *cur = *head;
	int count = 0;
	while (cur->next)
	{
		cur = cur->next;
		count++;
	}
	return count;
}