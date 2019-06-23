#include <stdio.h>
extern char** environ;

extern "C" {
  extern int execve(char const *path, char* const *argv, char* const *envp);
  int execv(char const *path, char* const *argv) {
    printf("--------------------------- %s\n", path);

    char*const* argv1 = argv;
    while (*argv1) {
      puts(*argv1++);
    }

    char** env = environ;
    while (*env) {
      puts(*env++);
    }

    return execve(path, argv, environ);
  }
}
