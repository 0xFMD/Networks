#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT 6555

void wav_header(FILE *file, size_t data_size) {
  uint32_t file_size = 36 + data_size;
  uint32_t fmt_size = 16;
  uint16_t format = 1;
  uint16_t channels = 1;
  uint32_t sample_rate = 44100;
  uint32_t byte_rate = 44100;
  uint16_t block_align = 1;
  uint16_t bits = 8;

  fwrite("RIFF", 1, 4, file);
  fwrite(&file_size, sizeof(file_size), 1, file);
  fwrite("WAVE", 1, 4, file);

  fwrite("fmt ", 1, 4, file);
  fwrite(&fmt_size, sizeof(fmt_size), 1, file);
  fwrite(&format, sizeof(format), 1, file);
  fwrite(&channels, sizeof(channels), 1, file);
  fwrite(&sample_rate, sizeof(sample_rate), 1, file);
  fwrite(&byte_rate, sizeof(byte_rate), 1, file);
  fwrite(&block_align, sizeof(block_align), 1, file);
  fwrite(&bits, sizeof(bits), 1, file);

  fwrite("data", 1, 4, file);
  fwrite(&data_size, sizeof(data_size), 1, file);
}

int main(void) {
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

  if (listen(server_fd, 2) < 0) {
    close(server_fd);
    return 1;
  }

  while (1) {
    int client_fd = accept(server_fd, NULL, NULL);

    time_t current_time = time(NULL);
    char filename[64];
    sprintf(filename, "voice_%ld.wav", current_time);
    FILE *audio = fopen(filename, "wb+");

    wav_header(audio, 0);

    char buf[1024];
    ssize_t n;
    while ((n = read(client_fd, buf, sizeof(buf))) > 0)
      fwrite(buf, 1, n, audio);

    rewind(audio);
    wav_header(audio, n);

    fclose(audio);
    close(client_fd);
  }

  close(server_fd);
}