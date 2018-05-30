all:
	gcc -g3 -o httpd server.c worker_server.c producer.c commands.c buflist.c valid_request.c -pthread
	gcc -g3 -o client client.c check_response.c worker_client.c buflist.c commands_client.c -pthread