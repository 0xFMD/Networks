#ifndef __C_CHAT_H__
#define __C_CHAT_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME_LENGTH 16
#define MAX_TEXT_BUF 1024

bool send_message(const int socket_fd, const char *text);

#endif
