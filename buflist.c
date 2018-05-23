#include "buflist.h"

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