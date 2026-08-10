#include "../include/client.h"

#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

int connect_to_server(const char *host, const uint16_t port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return -1;

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
  };

  if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0)
    return -1;

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    return -1;

  return fd;
}

int main(void) {
  int server_fd = connect_to_server(SERVER_IP, PORT);
  if (server_fd < 0)
    return 1;

  if (write(fileno(stdout), "\033[2J\033[H", 7) < 0)
    return 1;

  struct pollfd fds[2] = {
      {fileno(stdin), POLLIN, 0},
      {server_fd, POLLIN, 0},
  };

  while (true) {
    poll(fds, 2, -1);

    if (fds[0].revents & POLLIN) {
      char text[MAX_TEXT_BUF] = {0};

      if (read(fileno(stdin), text, sizeof(text) - 1) <= 0)
        break;

      send_message(server_fd, text);
    }

    if (fds[1].revents & POLLIN) {
      char text[MAX_TEXT_BUF] = {0};

      if (read(server_fd, text, sizeof(text) - 1) <= 0)
        break;

      if (write(fileno(stdout), text, strlen(text)) < 0)
        break;
    }
  }

  close(server_fd);
}