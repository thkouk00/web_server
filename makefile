OBJS1	= server.o worker_server.o producer.o commands.o buflist.o valid_request.o
OBJS2	= client.o check_response.o worker_client.o commands_client.o buflist.o
SOURCE1	= server.c worker_server.c producer.c commands.c buflist.c valid_request.c 
SOURCE2	= client.c check_response.c worker_client.c commands_client.c buflist.c
HEADER1	= server.h buflist.h valid_request.h 
HEADER2	= client.h check_response.h buflist.h
CC 	= gcc
FLAGS	= -c

all:	$(OBJS1) $(OBJS2)
	@cd ../jobExecutor && make
	$(CC) -g3 -o httpd $(OBJS1)	-pthread
	$(CC) -g3 -o client $(OBJS2) -pthread

	

server.o:	server.c 
	$(CC) $(FLAGS) server.c

worker_server.o:	worker_server.c	
	$(CC) $(FLAGS) worker_server.c

producer.o:	producer.c 
	$(CC) $(FLAGS) producer.c 

commands.o:	commands.c
	$(CC) $(FLAGS) commands.c

buflist.o:	buflist.c 
	$(CC) $(FLAGS) buflist.c

valid_request.o:	valid_request.c 
	$(CC) $(FLAGS) valid_request.c

client.o:	client.c 
	$(CC) $(FLAGS) client.c

check_response.o:	check_response.c 
	$(CC) $(FLAGS) check_response.c

worker_client.o:	worker_client.c 
	$(CC) $(FLAGS) worker_client.c

commands_client.o:	commands_client.c 
	$(CC) $(FLAGS) commands_client.c

clean:
	rm -f $(OBJS1) $(OBJS2) httpd client