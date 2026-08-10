#include "../include/c_chat.h"
#include <sys/socket.h>
#include <unistd.h>

bool send_message(const int socket_fd, const char *text) {
  if (socket_fd < 0 || !text)
    return false;

  size_t length = strlen(text);
  ssize_t sent = write(socket_fd, text, length);

  return sent == length;
}
