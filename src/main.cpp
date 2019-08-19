#include "vfs.hpp"
#include "functions.hpp"

// for emacs
#ifndef SERVER_BUILD
#define SERVER_BUILD
#endif
#include "server.hpp"
#include "log.h"
#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (false) {
    char** env = environ;
    while (*env) {
      puts(*env++);
    }
  }

  char self_path[PATH_MAX];
  self_path[0] = 0;
  ssize_t bytes = winevfs__readlink("/proc/self/exe", self_path, PATH_MAX - 1);
  if (bytes <= 0) {
    puts("Unable to find /proc/self/exe");
    return -1;
  } else {
    self_path[bytes] = 0;
  }

  std::filesystem::path parent_path = self_path;
  parent_path = parent_path.parent_path().parent_path() / "$LIB/libwinevfs_lib.so";
  setenv("LD_PRELOAD", parent_path.c_str(), true);

  char* envfile = getenv("WINEVFS_VFSFILE");
  if (envfile && envfile[0]) {
    std::filesystem::path path = winevfs_abspath(envfile);
    setenv("WINEVFS_VFSFILE", path.c_str(), true);
  }

  if (argc < 2) {
    return 0;
  }

  char* args[argc];
  memcpy(args, argv + 1, (argc - 1) * sizeof(char*));
  args[argc - 1] = NULL;

  //puts("[winevfs] Running");

  return execvpe(args[0], args, environ);
}
