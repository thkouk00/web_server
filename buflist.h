#ifndef _BUFLISTH_
#define _BUFLISTH_
#include <stdio.h>
#include <stdlib.h>

typedef struct buf_struct{
	int fd;
	struct buf_struct *next;
} buflist;

void push(buflist** , int);
void pop_head(buflist**,int*);
void print(buflist**);
void freelist(buflist**);


#endif