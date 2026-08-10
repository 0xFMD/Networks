#include <alsa/asoundlib.h>
#include <arpa/inet.h>
#include <linux/videodev2.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>

#define PORT 6567
#define AUDIO_PORT PORT + 1

typedef struct {
  void *data;
  struct v4l2_buffer info;
} vid_buffer_t;

int create_server(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_port = htons(port),
                             .sin_addr.s_addr = htonl(INADDR_ANY)};

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "bind");
    return -1;
  }

  if (listen(fd, 2) < 0) {
    fprintf(stderr, "listen");
    return -1;
  }

  return fd;
}

void init_cam(int cam_fd, vid_buffer_t *buf) {

  struct v4l2_format fmt = {.type = V4L2_BUF_TYPE_VIDEO_CAPTURE};

  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  ioctl(cam_fd, VIDIOC_S_FMT, &fmt);

  struct v4l2_requestbuffers req = {.count = 1,
                                    .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
                                    .memory = V4L2_MEMORY_MMAP};
  ioctl(cam_fd, VIDIOC_REQBUFS, &req);

  buf->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf->info.memory = V4L2_MEMORY_MMAP;
  buf->info.index = 0;
  ioctl(cam_fd, VIDIOC_QUERYBUF, &buf->info);

  buf->data = mmap(NULL, buf->info.length, PROT_READ, MAP_SHARED, cam_fd,
                   buf->info.m.offset);

  if (buf->data == MAP_FAILED)
    return;

  ioctl(cam_fd, VIDIOC_QBUF, &buf->info);

  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(cam_fd, VIDIOC_STREAMON, &type);
}

void stream_audio(void) {
  const int server_fd = create_server(AUDIO_PORT);
  if (server_fd < 0)
    return;
  int client_fd = accept(server_fd, NULL, NULL);

  if (client_fd < 0)
    return;

  snd_pcm_t *pcm;
  snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);

  snd_pcm_set_params(pcm, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED, 1,
                     44100, 1, 100000);

  char buf[1024];

  while (1) {
    int frames = snd_pcm_readi(pcm, buf, 1024);

    if (write(client_fd, buf, frames) < 0)
      break;
  }
}

int main(void) {
  pid_t p_audio = fork();

  if (p_audio == 0) {
    stream_audio();
    return 0;
  }

  const int cam_fd = open("/dev/video0", O_RDWR);
  if (cam_fd < 0)
    return 1;

  vid_buffer_t buf = {0};

  init_cam(cam_fd, &buf);

  const int server_fd = create_server(PORT);
  if (server_fd < 0)
    return 1;

  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0)
    return 1;

  while (1) {
    ioctl(cam_fd, VIDIOC_DQBUF, &buf.info);
    if (write(client_fd, buf.data, buf.info.bytesused) < 0)
      break;
    ioctl(cam_fd, VIDIOC_QBUF, &buf.info);
  }

  close(client_fd);
  close(server_fd);
  close(cam_fd);
}