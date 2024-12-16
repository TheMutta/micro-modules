#pragma once
#include <stdio.h>
#include <stdarg.h>

#define SOCKET_NAME "talk.sock"

typedef enum {
	MSG_TYPE_PRINT,
	MSG_TYPE_QUIT,
} message_type_t;

typedef struct {
	message_type_t type;
	size_t size;
	char data[];
} __attribute__((packed)) message_t;

void module_log(const char *module, char *format, ...) {
	va_list ap;
	va_start(ap, format);
	printf("[%s] ", module);
	vprintf(format, ap);
	va_end(ap);
}
