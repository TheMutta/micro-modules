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

#define SOCKET_NAME "talk.sock"

static int sockfd;
static struct sockaddr_un addr;
static bool data_sent;

int module_init() {
	module_log("MODULE B", "Hello, world A!\n");

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

		const char *data = "Hello, module!";
		size_t size = sizeof(message_t) + strlen(data);
		message_t *message = malloc(size);
		message->type = MSG_TYPE_PRINT;
		message->size = strlen(data);
		strcpy(message->data, data);
		send(sockfd, message, size, MSG_DONTWAIT);
		data_sent = true;
	}

	return 0;
}

void module_deinit() {
	module_log("MODULE B", "Bye, world B!\n");

	close(sockfd);
}
