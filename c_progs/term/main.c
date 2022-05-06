#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int main(void) {
  const char device[] = "/dev/ttys0";

  int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1) {
    fprintf(stderr, "failed to open port\n");
    return -1;
  }

  // check if fd is pointing to a TTY device
  if (!isatty(fd)) {
    fprintf(stderr, "not a tty device\n");
    return -1;
  }

  struct termios or_tm;

  // load the config for the current serial interface
  if (tcgetattr(fd, &or_tm) < 0) {
    fprintf(stderr, "could not get serial interface\n");
    return -1;
  }

  // input flags
  // convert break to null byte, no CR to NL translation
  or_tm.c_iflag &=
      ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);

  // output flags
  // or_tm.c_oflag = 0;

  or_tm.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | ONOEOT | OFILL | OPOST);

  // no line processing
  or_tm.c_cflag &= ~(CSIZE | PARENB);

  or_tm.c_cflag |= CS8;

  // one input byte is enough to return from read()
  or_tm.c_cc[VMIN] = 1;
  or_tm.c_cc[VTIME] = 0;

  // communication speed
  if (cfsetispeed(&or_tm, B9600) < 0 || cfsetospeed(&or_tm, B9600) < 0) {
    fprintf(stderr, "could not set speed\n");
    return -1;
  }

  // apply config
  if (tcsetattr(fd, TCSAFLUSH, &or_tm) < 0) {
    fprintf(stderr, "could not apply the config\n");
    return -1;
  }

  close(fd);
}
