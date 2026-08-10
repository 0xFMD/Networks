#ifndef __C_CHAT_H__
#define __C_CHAT_H__

#include <cstdint>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME_LENGTH 64
#define MAX_MESSAGE_LENGTH 512
#define IP_MAX 512

typedef struct user_t user_t;
typedef struct payload_t payload_t;

struct user_t {
  char username[MAX_USERNAME_LENGTH];
  char ip[IP_MAX];
};

typedef enum { JOIN, MESSAGE, LEAVE } payload_type_t;

struct payload_t {
  user_t user_metadata;
  payload_type_t type;
  uint32_t message_len;
  char payload[MAX_MESSAGE_LENGTH];
};

user_t *create_user(const char *username, const char *ip);
payload_t *create_message(const user_t *user, const char *message);

bool edit_message(payload_t *message);

void delete_user(user_t *user);
void delete_payload(payload_t *message);

int init_socket();

bool send_message(int socket_fd, const payload_t *message);

#endif
