all:
	gcc -o httpd server.c buflist.c valid_request.c -pthread
	gcc -o client client.c