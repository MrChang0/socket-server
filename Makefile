socket-server : socket_server.c test.c uart_server.c
	gcc -g -Wall -o $@ $^ -lpthread

clean:
	rm socket-server
