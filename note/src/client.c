#include <alsa/asoundlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>

#define PORT 6555
#define SERVER_IP "127.0.0.1"

int connect_server() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_port = htons(PORT),
                             .sin_addr.s_addr = inet_addr(SERVER_IP)};

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  };

  return fd;
}

FILE *record_audio() {
  snd_pcm_t *pcm;

  if (snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0) < 0)
    return NULL;

  if (snd_pcm_set_params(pcm, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED,
                         1, 44100, 1, 100000) < 0) {
    snd_pcm_close(pcm);
    return NULL;
  }

  FILE *audio = tmpfile();

  if (!audio) {
    snd_pcm_close(pcm);
    return NULL;
  }

  char buf[1024];
  int input;

  struct pollfd keyboard = {.fd = STDIN_FILENO, .events = POLLIN};

  printf("Recording, enter 1 to stop\n");

  while (1) {
    if (poll(&keyboard, 1, 0) > 0) {
      if (scanf("%d", &input) != 1)
        continue;

      if (input == 1)
        break;
    }

    int frames = snd_pcm_readi(pcm, buf, sizeof(buf));
    fwrite(buf, 1, frames, audio);
  }
  snd_pcm_close(pcm);
  printf("Stopped\n");

  return audio;
}

int main(void) {
  FILE *audio = NULL;
  int input_choice;

  while (1) {
    printf("\n[1] Record\n");
    printf("[2] Send\n");
    printf("[3] Exit\n");
    printf("> ");

    if (scanf("%d", &input_choice) != 1)
      continue;

    if (input_choice == 1) {
      if (audio)
        fclose(audio);

      audio = record_audio();
    }

    else if (input_choice == 2) {
      if (!audio)
        continue;

      int server_fd = connect_server();
      if (server_fd < 0)
        continue;

      rewind(audio);
      char buf[1024];
      int n;

      while ((n = fread(buf, 1, sizeof(buf), audio)) > 0)
        if (write(server_fd, buf, n) < 0)
          break;

      close(server_fd);
      printf("Sent\n");
    } else if (input_choice == 3)
      return 0;
  }

  fclose(audio);

  return 0;
}