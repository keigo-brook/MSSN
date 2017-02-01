#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define BAUD_RATE B9600
#define BUFF_SIZE 4096

void serial_init(int fd) {
  struct termios tio;
  memset(&tio, 0, sizeof(tio));
  tio.c_cflag = CS8 | CLOCAL | CREAD;
  tio.c_cc[VTIME] = 0;
  // set baud rate
  cfsetispeed(&tio, BAUD_RATE);
  cfsetospeed(&tio, BAUD_RATE);
  // set to device
  tcsetattr(fd, TCSANOW, &tio);
}

int main(int argc, char *argv[]) {
  unsigned char buffer[BUFF_SIZE], in_data[BUFF_SIZE];
  /* printf("start serial port read\n"); */

  // open device file (serial port)
  char *dev_name = argv[1];
  if (dev_name == NULL) {
    printf("ARGV[1](DEV_NAME) is NULL\n");
    exit(1);
  }

  int fd = open(dev_name, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    printf("ERROR on device open\n");
    exit(1);
  }

  /* printf("init serial port\n"); */
  serial_init(fd);

  /* printf("start main loop\n"); */
  int i = 0, k = 0;
  struct timeval timer;

  while (1) {
    int len = read(fd, buffer, BUFF_SIZE);
    if (len == 0) {
      continue;
    }

    // drop initial data
    if (k < 15) {
      k++;
      continue;
    }

    if (len < 0) {
      printf("ERROR: %d\n", len);
      perror("");
      exit(2);
    }

    gettimeofday(&timer, NULL);
    for (int j = 0; j < len; j++) {
      in_data[i] = buffer[j];
      i++;
      if (buffer[j] == '\n') {
        in_data[i] = '\0';
        printf("%ld.%d %s", timer.tv_sec, timer.tv_usec, &in_data[0]);
        /* printf("%s", &in_data[0]); */
        fflush(stdout);
        i = 0;
        memset(in_data, 0, BUFF_SIZE);
      }
    }
    k++;
    memset(buffer, 0, BUFF_SIZE);
  }
}
