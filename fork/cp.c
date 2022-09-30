#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "errors.h"

int main(int argc, char *argv[]) {
  int dest_fd;
  int src_fd = open(argv[1], O_RDONLY);

  if (src_fd < 0) {
    perror("Error: ");
    return -1;
  }
  if ((dest_fd = open(argv[2], O_RDONLY)) > 0) {
    fprintf(stderr, "Error: Destination file already exists\n");
    close_checking(src_fd);
    close_checking(dest_fd);
    return -1;
  }

  dest_fd = open(argv[2], O_RDWR | O_CREAT, 0666);

  if (dest_fd < 0) {
    perror("Error: ");
    close_checking(src_fd);
    return -1;
  }

  int filesize = lseek(src_fd, 0, SEEK_END);
  ftruncate(dest_fd, filesize);

  char *src = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0);

  char *dest = mmap(NULL, filesize, PROT_WRITE, MAP_SHARED, dest_fd, 0);

  memcpy(dest, src, filesize);

  munmap(dest, filesize);
  munmap(src, filesize);

  close_checking(src_fd);
  close_checking(dest_fd);

  return 0;
}
