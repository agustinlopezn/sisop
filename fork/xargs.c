#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef NARGS
#define NARGS 4
#endif

// Resets all the parameters in function from the argument start (always > 1 to
// not delete the function to call)
void reset_function_parameters(char **function, int start) {
  for (int i = start; i < NARGS + 2; i++) {
    function[i] = NULL;
  }
}

void add_parameters(char **function, char *parameter, int *line_number,
                    int read) {
  parameter[read - 1] = '\0';
  *line_number += 1;
  function[*line_number] = parameter;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Error: wrong number of arguments");
    return -1;
  }
  char *function[NARGS + 2];  // function to execute + arguments + NULL
  char *buffer = NULL;
  size_t bufsize = 0, read = 0;
  int line_number = 0;
  function[0] = argv[1];

  reset_function_parameters(function, 1);

  while ((read = getline(&buffer, &bufsize, stdin))) {
    if (read == EOF || (line_number + 1 == NARGS)) {
      if (read != EOF) {
        add_parameters(function, buffer, &line_number, read);
      }

      int i = fork();

      if (i < 0) {
        perror("Error");
        return -1;
      }

      if (i == 0) {
        if (execvp(function[0], function) < 0) {
          perror("Error");
          return -1;
        }
      } else {
        wait(NULL);
      }

      line_number = 0;
      reset_function_parameters(function, 1);
      if (read == EOF) break;
    } else {
      add_parameters(function, buffer, &line_number, read);
      buffer = NULL;
    }
  }

  return 0;
}
