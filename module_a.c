#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#include "module.h"

static int sockfd;
static int clientfd;
static int poll_count;
static struct pollfd *fds;
static struct sockaddr_un addr, peer_addr;

int module_init() {
	module_log("MODULE A", "Hello, world A!\n");

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
	
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKET_NAME);
	unlink(SOCKET_NAME);
	assert(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) >= 0);
	
	assert(listen(sockfd, 1) >= 0);

	fds = calloc(2, sizeof(struct pollfd));

	poll_count = 1;
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	return 0;
}

int module_run() {
	int ret = poll(fds, poll_count, 0);

	if (ret >= 1) {
		if(fds[0].revents & POLLIN) {
			socklen_t peer_addr_size = sizeof(peer_addr);
			clientfd = accept(sockfd, (struct sockaddr*)&peer_addr, &peer_addr_size);

			fds[poll_count].fd = clientfd;
			fds[poll_count].events = POLLIN;
			++poll_count;
		} else if (fds[0].revents & (POLLERR | POLLHUP)) {
		}
		
		for (int i = 1; i < poll_count; ++i) {
			if(fds[i].revents & POLLIN) {
				message_t message;
				recv(fds[i].fd, &message, sizeof(message), MSG_DONTWAIT);

				switch(message.type) {
					case MSG_TYPE_PRINT: {
						char buffer[message.size];
						memset(buffer, 0, message.size);
						recv(fds[i].fd, buffer, message.size, MSG_DONTWAIT);

						module_log("MODULE A", "Received data: %s\r\n", buffer);
						}
						break;
					case MSG_TYPE_QUIT: {
						return -1;
						}
						break;
				}
			}
		}
	}

	return 0;
}

void module_deinit() {
	module_log("MODULE A", "Bye, world A!\n");

	close(clientfd);
	close(sockfd);

	unlink(SOCKET_NAME);
}
