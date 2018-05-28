#ifndef _BUFLISTH_
#define _BUFLISTH_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//for server
typedef struct buf_struct{
	int fd;
	struct buf_struct *next;
} buflist;

void push(buflist** , int);
void pop_head(buflist**,int*);
void print(buflist**);
void freelist(buflist**);

//for client
typedef struct client_struct{
	char* url;
	struct client_struct *next;
} url_queue;

void push_c(url_queue** , char*);
void pop_head_c(url_queue**,char**);
int search_c(url_queue**, char*);
int urls_left(url_queue**);
void print_c(url_queue**);
void freelist_c(url_queue**);

#endif