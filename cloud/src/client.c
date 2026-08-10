#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 6567

enum { UPLOAD, DOWNLOAD, VIEW };

typedef struct {
  int action;
  char filename[128];
  size_t size;
} payload_t;

int connect_server(void) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(PORT),
      .sin_addr.s_addr = inet_addr("127.0.0.1"),
  };

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

void upload(char *filename) {
  FILE *file = fopen(filename, "rb");

  if (!file) {
    printf("File not found\n");
    return;
  }

  struct stat st;

  if (stat(filename, &st) < 0) {
    fclose(file);
    return;
  }

  int fd = connect_server();

  if (fd < 0) {
    fclose(file);
    return;
  }

  payload_t payload = {
      .action = UPLOAD,
      .size = st.st_size,
  };

  snprintf(payload.filename, sizeof(payload.filename), "%s", filename);

  write(fd, &payload, sizeof(payload));

  char buf[4096];
  size_t n;

  while ((n = fread(buf, 1, sizeof(buf), file)) > 0)
    write(fd, buf, n);

  fclose(file);
  close(fd);
}

void download(char *filename) {
  int fd = connect_server();

  payload_t payload = {.action = DOWNLOAD};

  strcpy(payload.filename, filename);

  write(fd, &payload, sizeof(payload));

  if (read(fd, &payload, sizeof(payload)) <= 0) {
    printf("File not found\n");
    close(fd);
    return;
  }
  

  FILE *file = fopen(filename, "wb");

  char buf[4096];

  while (payload.size > 0) {
    int n = read(fd, buf, sizeof(buf));

    if (n <= 0)
      break;

    fwrite(buf, 1, n, file);
    payload.size -= n;
  }

  fclose(file);
  close(fd);
}

void view(void) {
  int fd = connect_server();

  if (fd < 0)
    return;

  payload_t payload = {.action = VIEW};

  write(fd, &payload, sizeof(payload));

  printf("\nFiles:\n");

  while (read(fd, &payload, sizeof(payload)) > 0)
    printf("%s\n", payload.filename);

  close(fd);
}

int main(void) {
  int choice;
  char filename[128];

  while (1) {
    printf("\n[1] Upload\n");
    printf("[2] Download\n");
    printf("[3] View\n");
    printf("[4] Exit\n");
    printf("> ");

    scanf("%d", &choice);

    if (choice == 1) {
      printf("File: ");
      scanf("%s", filename);
      upload(filename);
    }

    else if (choice == 2) {
      printf("File: ");
      scanf("%s", filename);
      download(filename);
    }

    else if (choice == 3)
      view();

    else if (choice == 4)
      break;
  }

  return 0;
}