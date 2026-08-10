#ifndef __SERVER_H__
#define __SERVER_H__
#include "./c_chat.h"

#define MAX_CLIENTS 16

struct client_t {
  int fd;
  bool joined;
  user_t user;
};

static client_t clients[MAX_CLIENTS];
static size_t client_count;

int listen(const uint16_t port);
bool broadcast(const payload_t *msg, const int except_fd);

#endif