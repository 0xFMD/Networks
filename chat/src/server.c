#include "../include/server.h"
#include <poll.h>

client_t CLIENTS[MAX_CLIENTS];

int server_listen(const uint16_t port) {
  for (size_t i = 0; i < MAX_CLIENTS; i++)
    CLIENTS[i].fd = -1;

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return -1;

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_port = htons(port),
                             .sin_addr.s_addr = htonl(INADDR_ANY)};

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    return -1;

  if (listen(fd, MAX_CLIENTS) < 0)
    return -1;

  return fd;
}

void broadcast(const char *text, const int except_fd) {
  for (size_t i = 0; i < MAX_CLIENTS; i++) {
    if (CLIENTS[i].fd == -1 || !CLIENTS[i].is_registered ||
        CLIENTS[i].fd == except_fd)
      continue;

    send_message(CLIENTS[i].fd, text);
  }
}

void broadcast_users() {
  char text[MAX_TEXT_BUF] = "\033[s\033[H\033[2Kusers: ";

  for (size_t i = 0; i < MAX_CLIENTS; i++) {
    if (!CLIENTS[i].is_registered)
      continue;

    strcat(text, CLIENTS[i].username);
    strcat(text, " ");
  }

  strcat(text, "\n\033[2K===================\033[u");

  broadcast(text, -1);
}

bool is_name_taken(const char *username) {
  for (size_t i = 0; i < MAX_CLIENTS; i++) {
    if (!CLIENTS[i].is_registered)
      continue;

    if (strcmp(CLIENTS[i].username, username) == 0)
      return true;
  }

  return false;
}

void accept_client(const int server_fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);

  int client_fd = accept(server_fd, (struct sockaddr *)&addr, &len);
  if (client_fd < 0)
    return;

  for (size_t i = 0; i < MAX_CLIENTS; i++) {
    if (CLIENTS[i].fd != -1)
      continue;

    CLIENTS[i].fd = client_fd;
    CLIENTS[i].is_registered = false;

    sprintf(CLIENTS[i].ip, "%s", inet_ntoa(addr.sin_addr));

    send_message(client_fd, "Username: ");
    return;
  }

  send_message(client_fd, "Server full\n");
  close(client_fd);
}

void register_client(const size_t i, const char *username) {
  if (is_name_taken(username)) {
    send_message(CLIENTS[i].fd, "Username taken\n");
    return;
  }

  sprintf(CLIENTS[i].username, "%s", username);
  CLIENTS[i].is_registered = true;

  send_history(CLIENTS[i].fd);

  char text[MAX_TEXT_BUF];
  sprintf(text, "----%s joined----\n", CLIENTS[i].username);

  broadcast(text, CLIENTS[i].fd);
  save_history(text);
  broadcast_users();
}

void remove_client(size_t i) {
  if (CLIENTS[i].is_registered) {
    char text[MAX_TEXT_BUF];
    sprintf(text, "----%s left----\n", CLIENTS[i].username);
    save_history(text);
    broadcast(text, CLIENTS[i].fd);
  }

  close(CLIENTS[i].fd);

  CLIENTS[i].fd = -1;
  CLIENTS[i].is_registered = false;

  broadcast_users();
}

void save_history(const char *text) {
  FILE *file = fopen(HISTORY_FILE_NAME, "a");
  if (!file)
    return;

  fwrite(text, 1, strlen(text), file);
  fclose(file);
}

void send_history(int fd) {
  FILE *file = fopen(HISTORY_FILE_NAME, "r");
  if (!file)
    return;

  char text[MAX_TEXT_BUF] = {0};
  fread(text, 1, sizeof(text) - 1, file);
  fclose(file);

  send_message(fd, text);
}

int main(void) {
  int server_fd = server_listen(PORT);
  if (server_fd < 0)
    return 1;

  while (true) {
    struct pollfd fds[MAX_CLIENTS + 1];

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (size_t i = 0; i < MAX_CLIENTS; i++) {
      fds[i + 1].fd = CLIENTS[i].fd;
      fds[i + 1].events = POLLIN;
    }

    poll(fds, MAX_CLIENTS + 1, -1);

    if (fds[0].revents & POLLIN)
      accept_client(server_fd);

    for (size_t i = 0; i < MAX_CLIENTS; i++) {
      if (!(fds[i + 1].revents & POLLIN))
        continue;

      char text[MAX_TEXT_BUF] = {0};

      if (read(CLIENTS[i].fd, text, sizeof(text) - 1) <= 0) {
        remove_client(i);
        continue;
      }

      text[strcspn(text, "\r\n")] = '\0';

      if (!CLIENTS[i].is_registered) {
        register_client(i, text);
        continue;
      }

      char message[MAX_TEXT_BUF + MAX_USERNAME_LENGTH];
      sprintf(message, "%s: %s\n", CLIENTS[i].username, text);

      save_history(message);
      broadcast(message, CLIENTS[i].fd);
    }
  }
}