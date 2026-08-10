#include <arpa/inet.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 6567
#define ROOT "bucket"

enum { UPLOAD, DOWNLOAD, VIEW };

typedef struct {
  int action;
  char filename[128];
  size_t size;
} payload_t;

bool valid_path(char *path) {
  if (path[0] == '/')
    return false;

  if (strstr(path, ".."))
    return false;

  return true;
}

void make_dirs(char *path) {
  int i = 0;

  while (path[i]) {
    if (path[i] == '/') {
      path[i] = '\0';
      mkdir(path, 0755);
      path[i] = '/';
    }

    i++;
  }
}

void upload(int fd, payload_t payload) {
  if (!valid_path(payload.filename))
    return;

  char path[256];
  sprintf(path, "%s/%s", ROOT, payload.filename);

  make_dirs(path);

  FILE *file = fopen(path, "wb");

  if (!file) {
    payload.size = -1;
    write(fd, &payload, sizeof(payload));
    return;
  }

  char buf[4096];

  while (payload.size > 0) {
    int n = read(fd, buf, sizeof(buf));

    if (n <= 0)
      break;

    fwrite(buf, 1, n, file);
    payload.size -= n;
  }

  fclose(file);
}

void download(int fd, payload_t payload) {
  if (!valid_path(payload.filename))
    return;

  char path[256];
  sprintf(path, "%s/%s", ROOT, payload.filename);

  FILE *file = fopen(path, "rb");

  if (!file)
    return;

  struct stat st;
  stat(path, &st);

  payload.size = st.st_size;

  write(fd, &payload, sizeof(payload));

  char buf[4096];
  int n;

  while ((n = fread(buf, 1, sizeof(buf), file)) > 0)
    write(fd, buf, n);

  fclose(file);
}

void view(int fd) {
  DIR *dir = opendir(ROOT);
  struct dirent *entry;

  payload_t payload = {0};

  while ((entry = readdir(dir))) {
    if (entry->d_name[0] == '.')
      continue;

    strcpy(payload.filename, entry->d_name);
    write(fd, &payload, sizeof(payload));
  }

  closedir(dir);
}

int main(void) {
  mkdir(ROOT, 0755);

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(PORT),
      .sin_addr.s_addr = htonl(INADDR_ANY),
  };

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(server_fd);
    return 1;
  };
  if (listen(server_fd, 5) < 0) {
    close(server_fd);
    return 1;
  };

  while (1) {
    int fd = accept(server_fd, NULL, NULL);

    payload_t payload = {0};

    if (read(fd, &payload, sizeof(payload)) <= 0) {
      close(fd);
      continue;
    }

    if (payload.action == UPLOAD)
      upload(fd, payload);

    else if (payload.action == DOWNLOAD)
      download(fd, payload);

    else if (payload.action == VIEW)
      view(fd);

    close(fd);
  }
}