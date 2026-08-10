#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 6567
#define AUDIO_PORT PORT + 1
#define SERVER_IP "127.0.0.1"

int connect_server(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0)
    return -1;

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
  };

  inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

void play_audio(void) {
  int fd = connect_server(AUDIO_PORT);

  if (fd < 0)
    return;

  FILE *ffplay = popen("ffplay "
                       "-nodisp "
                       "-f u8 "
                       "-ar 44100 "
                       "-ch_layout mono "
                       "-i -",
                       "w");

  if (!ffplay) {
    close(fd);
    return;
  }

  char buf[1024];
  ssize_t n;

  while ((n = read(fd, buf, sizeof(buf))) > 0)
    fwrite(buf, 1, n, ffplay);

  pclose(ffplay);
  close(fd);
}

void play_video(void) {
  int fd = connect_server(PORT);

  if (fd < 0)
    return;

  FILE *ff = popen("ffplay "
                   "-f rawvideo "
                   "-pixel_format yuyv422 "
                   "-video_size 640x480 "
                   "-framerate 30 "
                   "-i -",
                   "w");

  if (!ff) {
    close(fd);
    return;
  }

  char buf[4096];
  ssize_t n;

  while ((n = read(fd, buf, sizeof(buf))) > 0)
    fwrite(buf, 1, n, ff);

  pclose(ff);
  close(fd);
}

int main(void) {
  pid_t audio_p = fork();

  if (audio_p == 0) {
    play_audio();
    return 0;
  }

  play_video();

  return 0;
}