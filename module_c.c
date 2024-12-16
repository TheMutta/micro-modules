#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>

#include "module.h"

static int sockfd;
static struct sockaddr_un addr;
static bool data_sent;

int module_init() {
	module_log("MODULE C", "Hello, world C!\n");

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKET_NAME);

	data_sent = false;

	return 0;
}

int module_run() {
	if (!data_sent) {
		int result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

		if (result == -EAGAIN) {
			return 0;
		}	

		const char *data = "Hello, world!";
		size_t size = sizeof(message_t) + strlen(data);
		message_t *message = malloc(size);
		message->type = MSG_TYPE_PRINT;
		message->size = size;
		strcpy(message->data, data);
		send(sockfd, message, size, MSG_DONTWAIT);
		data_sent = true;
	} else {
		message_t message;
		message.type = MSG_TYPE_QUIT;
		message.size = 0;
		send(sockfd, &message, sizeof(message_t), MSG_DONTWAIT);

	}
	return 0;
}

void module_deinit() {
	module_log("MODULE C", "Bye, world C!\n");
	
	close(sockfd);
}
