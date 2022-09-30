#include <unistd.h>

void close_checking(int fd) {
  if (close(fd) < 0) perror("Error: ");
}