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
void push_c(url_queue** head, char* url)
{
	
	printf("Phra %s\n", url);
	// create head if doesn't exist
	// then insert node at end of list (queue implementation)
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
	memcpy(cur->url,url,strlen(url));
	cur->url[strlen(cur->url)] = '\0';
	cur->next = NULL;
}

// pop first node of queue
void pop_head_c(url_queue** head,char** url)
{
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

// print all nodes in queue
void print_c(url_queue** head)
{
	url_queue *cur = *head;
	if (cur == NULL)
		exit(1);
	while (cur->next)
	{
		cur = cur->next;
		printf("Url is %s\n", cur->url);
	}
}