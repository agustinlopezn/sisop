#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "errors.h"

int main(void) {
  int first_fds[2];
  int second_fds[2];

  pipe(first_fds);
  pipe(second_fds);

  int i = fork();

  if (i < 0) {
    perror("Error");
    return -1;
  }

  if (i == 0) {
    long int recv = 0;
    close_checking(second_fds[1]);
    close_checking(first_fds[0]);

    if (read(second_fds[0], &recv, sizeof(recv)) < 0) perror("Error");
    if (write(first_fds[1], &recv, sizeof(recv)) < 0) perror("Error");

    printf("Donde fork me devuelve %d:\n", i);
    printf("\t- getpid me devuelve %d\n", getpid());
    printf("\t- getppid me devuelve %d\n", getppid());
    printf("\t- recibo valor %ld vía fd=%d\n ", recv, second_fds[0]);
    printf("\t- reenvío valor en fd=%d y termino\n\n", first_fds[1]);

    close_checking(first_fds[1]);
    close_checking(second_fds[0]);

  } else {
    srandom(time(NULL));
    long int random_value = random();
    long int recv = 0;

    if (write(second_fds[1], &random_value, sizeof(random_value)) < 0)
      perror("Error");

    printf("Hola, soy PID %d:\n", getpid());
    printf("\t- primer pipe me devuelve: [%d, %d]\n", first_fds[0],
           first_fds[1]);
    printf("\t- segundo pipe me devuelve: [%d, %d]\n\n", second_fds[0],
           second_fds[1]);

    close_checking(first_fds[1]);
    close_checking(second_fds[0]);
    close_checking(second_fds[1]);

    printf("Donde fork me devuelve %d:\n", i);
    printf("\t- getpid me devuelve %d\n", getpid());
    printf("\t- getppid me devuelve %d\n", getppid());
    printf("\t- random me devuelve %ld\n", random_value);
    printf("\t- envío valor %ld a través de fd=%d\n\n", random_value,
           second_fds[1]);

    sleep(1);  // Espera a que el hijo se lance, llegue a leer y a escribir para
               // luego leer lo que manda el hijo.

    if (read(first_fds[0], &recv, sizeof(recv)) < 0) perror("Error");

    printf("Hola, de nuevo PID %d:\n", i);
    printf("\t- recibí valor %ld vía fd=%d\n\n", recv, first_fds[0]);

    close_checking(first_fds[0]);
  }

  return 0;
}
