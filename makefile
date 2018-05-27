all:
	gcc -o httpd server.c worker_server.c producer.c commands.c buflist.c valid_request.c -pthread
	gcc -o client client.c check_response.c worker_client.c buflist.c -pthread