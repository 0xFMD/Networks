#ifndef __SERVER_H__
#define __SERVER_H__
#include "./c_chat.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 6543
#define HISTORY_FILE_NAME "chat.history"
#define MAX_CLIENTS 16
#define IP_MAX 64

typedef struct {
  int fd;
  bool is_registered;
  char username[MAX_USERNAME_LENGTH];
  char ip[IP_MAX];
} client_t;

int server_listen(const uint16_t port);

void accept_client(const int server_fd);
void register_client(const size_t index, const char *username);
bool is_name_taken(const char *username);

void broadcast(const char *text, const int except_fd);
void broadcast_users();

void save_history(const char *text);
void send_history(const int client_fd);

#endif
