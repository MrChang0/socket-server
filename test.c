#include "socket_server.h"
#include "uart_server.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void *
_poll(void * ud) {
	struct socket_server *ss = ud;
	struct socket_message result;
	for (;;) {
		int type = socket_server_poll(ss, &result, NULL);
		// DO NOT use any ctrl command (socket_server_close , etc. ) in this thread.
		switch (type) {
		case SOCKET_EXIT:
			return NULL;
		case SOCKET_DATA:
			printf("message(%lu) [id=%d] size=%d\n",result.opaque,result.id, result.ud);
			free(result.data);
			break;
		case SOCKET_CLOSE:
			printf("close(%lu) [id=%d]\n",result.opaque,result.id);
			break;
		case SOCKET_OPEN:
			printf("open(%lu) [id=%d] %s\n",result.opaque,result.id,result.data);
			break;
		case SOCKET_ERROR:
			printf("error(%lu) [id=%d]\n",result.opaque,result.id);
			break;
		case SOCKET_ACCEPT:
			printf("accept(%lu) [id=%d %s] from [%d]\n",result.opaque, result.ud, result.data, result.id);
			break;
		}
	}
}

static void
test(struct socket_server *ss) {
	pthread_t pid;
	pthread_create(&pid, NULL, _poll, ss);

	//int c = socket_server_connect(ss,100,"127.0.0.1",80);
	//printf("connecting %d\n",c);
	//int l = socket_server_listen(ss,200,"127.0.0.1",8888,32);
	//printf("listening %d\n",l);
	//socket_server_start(ss,201,l);
	//int b = socket_server_bind(ss,300,1);
	//printf("binding stdin %d\n",b);
	//int i;
	//for (i=0;i<100;i++) {
	//	socket_server_connect(ss, 400+i, "127.0.0.1", 8888);
	//}
	//sleep(5);
	int u = uart_server_open(ss,100,"/dev/ttyS1");
	if (u < 0) {
		goto fail__;
	}
	sleep(1);
	// sometimes set failure
	int ok = uart_server_set(ss, u, 115200, 0, 8, 1, 'N');
	if (ok)
	{
		int count = 5;
		while (count--)
		{
			char *buffer = malloc(5);
			memcpy(buffer, "hello", 5);
			uart_server_send(ss, u, buffer, 5);
			sleep(5);
		}
		uart_server_close(ss, 200, u);
	}
fail__:
	socket_server_exit(ss);

	pthread_join(pid, NULL); 
}

int
main() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);

	struct socket_server * ss = socket_server_create();
	test(ss);
	socket_server_release(ss);

	return 0;
}
