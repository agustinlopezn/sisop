#define _GNU_SOURCE
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "errors.h"

#define PATH_OFFSET 7

int main(int argc, char *argv[]) {
  struct dirent *entry;
  char aux[PATH_MAX];
  DIR *directory = opendir("/proc");
  int current_fd = dirfd(directory);

  char path[PATH_MAX];

  while ((entry = readdir(directory))) {
    int number = 0, i = 0;
    char process[PATH_MAX];
    memset(process, 0, PATH_MAX);
    memset(path, 0, PATH_MAX);

    for (i = 0; entry->d_name[i]; i++) {
      number += isdigit(entry->d_name[i]);
    }
    if (!number) continue;

    snprintf(path, i + PATH_OFFSET, "/proc/%s", entry->d_name);

    int next_fd = openat(current_fd, entry->d_name, O_RDONLY);
    int comm_fd = openat(next_fd, "comm", O_RDONLY);

    if (next_fd < 0 || comm_fd < 0) perror("Error: ");

    if (read(comm_fd, process, PATH_MAX) < 0) perror("Error: ");
    fprintf(stdout, " %s %s", entry->d_name, process);

    close_checking(comm_fd);
    close_checking(next_fd);
  }

  closedir(directory);
  return 0;
}
