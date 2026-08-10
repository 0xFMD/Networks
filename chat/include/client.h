#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "./c_chat.h"

#define PORT 6543
#define SERVER_IP "127.0.0.1"

int connect_to_server(const char *host, const uint16_t port);

#endif
