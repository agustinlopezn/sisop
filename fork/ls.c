#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

char *MONTHS[12] = {"jan", "feb", "mar", "apr", "may", "jun",
                    "jul", "aug", "sep", "oct", "nov", "dec"};

void read_directories(DIR *directory) {
  char *name = malloc(PATH_MAX);
  char file_name[PATH_MAX], linkbuf[PATH_MAX], file_type[PATH_MAX];
  struct dirent *entry;
  struct stat statbuf;
  struct tm lt, *ptm;
  time_t t;

  memset(linkbuf, 0, PATH_MAX);

  while ((entry = readdir(directory))) {
    if (stat(entry->d_name, &statbuf) < 0) perror("Error:");
    t = statbuf.st_mtime;
    ptm = localtime_r(&t, &lt); // formats the st_time 

    switch (statbuf.st_mode & S_IFMT) {
      case S_IFBLK:
        strcpy(file_type, "block device");
        break;
      case S_IFCHR:
        strcpy(file_type, "character device");
        break;
      case S_IFDIR:
        strcpy(file_type, "directory");
        break;
      case S_IFIFO:
        strcpy(file_type, "FIFO/pipe");
        break;
      case S_IFLNK:
        strcpy(file_type, "symlink");
        break;
      case S_IFREG:
        strcpy(file_type, "regular file");
        break;
      case S_IFSOCK:
        strcpy(file_type, "socket");
        break;
      default:
        strcpy(file_type, "unknown?");
        break;
    }

    if (entry->d_type == DT_LNK) {
      if (readlink(entry->d_name, linkbuf, 20) < 0) perror("Error:");
      snprintf(name, PATH_MAX, "%s -> %s", entry->d_name, linkbuf);
      strcpy(file_name, name);
    } else {
      strcpy(file_name, entry->d_name);
    }

    fprintf(stdout, " %14s %5d %4d %6ld %3s %2d %2d:%d %15s\n", file_type,
            statbuf.st_mode, statbuf.st_uid, statbuf.st_size,
            MONTHS[ptm->tm_mon], ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
            file_name);
  }

  free(name);
}

int main(void) {
  DIR *directory = opendir(".");

  read_directories(directory);

  closedir(directory);

  return 0;
}
