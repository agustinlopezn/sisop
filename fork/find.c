#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILTER_CASE_POS 1
#define FILTER_NON_CASE_POS 2

typedef char *(*compare_type)(const char *, const char *);

void read_directories(DIR *directory, char *buffer, char *filter,
                      compare_type compare) {
  struct dirent *entry;
  char aux[PATH_MAX];
  memset(aux, 0, PATH_MAX);
  strcpy(aux, buffer);

  if (strcmp(buffer, "./") == 0 && compare(buffer, filter))
    fprintf(stdout, "%s\n", buffer);

  while ((entry = readdir(directory))) {
    char *current_name = entry->d_name;
    if (strcmp(current_name, "..") == 0 || strcmp(current_name, ".") == 0)
      continue;

    if (entry->d_type == DT_DIR && compare(current_name, filter)) {
      int current_fd = dirfd(directory);
      int next_fd = openat(current_fd, current_name, O_DIRECTORY);

      fprintf(stdout, "%s%s\n", buffer, current_name);
      strcat(buffer, current_name);
      read_directories(fdopendir(next_fd), strcat(buffer, "/"), filter,
                       compare);
    } else if (compare(current_name, filter) || compare(buffer, filter)) {
      fprintf(stdout, "%s%s\n", aux, current_name);
    }
  }
}

int main(int argc, char *argv[]) {
  int opt = getopt(argc, argv, "i");
  DIR *directory = opendir(".");
  char buffer[PATH_MAX];

  memset(buffer, 0, PATH_MAX);
  strcat(buffer, "./");

  if (opt == 'i')
    read_directories(directory, buffer, argv[FILTER_NON_CASE_POS], &strcasestr);
  else
    read_directories(directory, buffer, argv[FILTER_CASE_POS], &strstr);

  closedir(directory);

  return 0;
}
