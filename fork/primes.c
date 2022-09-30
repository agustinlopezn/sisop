#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "errors.h"

#define MAX_NUMBER_POS 1

void generate_prime_process(int fds[2]) {
  unsigned int p = 0;
  int sub_fds[2];

  close_checking(fds[1]);
  if (read(fds[0], &p, sizeof(unsigned int)) <= 0) {
    close_checking(fds[0]);
    exit(0);
  }
  printf("primo %d\n", p);

  if (pipe(sub_fds) < 0) {
    perror("Error");
    return;
  }

  int i = fork();

  if (i < 0) {
    perror("Error");
    return;
  }

  if (i == 0) {
    close_checking(fds[0]);
    generate_prime_process(sub_fds);
    close_checking(sub_fds[0]);
    exit(0);
  } else {
    unsigned int n = 0;
    close_checking(sub_fds[0]);

    while (read(fds[0], &n, sizeof(unsigned int)) > 0) {
      if ((n % p) != 0) {
        if (write(sub_fds[1], &n, sizeof(n)) < 0) perror("Error");
      }
    }

    close_checking(sub_fds[1]);
    close_checking(fds[0]);
    wait(NULL);
    exit(0);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Error: wrong number of arguments");
    return -1;
  }
  unsigned int n = atoi(argv[MAX_NUMBER_POS]);
  int fds[2];
  pipe(fds);

  int i = fork();

  if (i < 0) {
    perror("Error");
    return -1;
  }

  if (i == 0) {
    generate_prime_process(fds);
  } else {
    for (unsigned int j = 2; j <= n; j++) {
      if (write(fds[1], &j, sizeof(unsigned int)) < 0) perror("Error");
    }
    close_checking(fds[0]);
    close_checking(fds[1]);
    int ret = wait(NULL);
  }

  return 0;
}
