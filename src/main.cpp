#include "vfs.hpp"
#include "functions.hpp"
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

  puts("[winevfs] Loading VFS file");
  winevfs_init();
  puts("[winevfs] Writing VFS file");
  winevfs_write_vfsfile("/tmp/.winevfs");
  setenv("WINEVFS_VFSFILE", "/tmp/.winevfs", true);

  char self_path[PATH_MAX];
  winevfs__readlink("/proc/self/exe", self_path, PATH_MAX);

  std::filesystem::path parent_path = self_path;
  parent_path = parent_path.parent_path().parent_path() / "$LIB/libwinevfs_lib.so";
  setenv("LD_PRELOAD", parent_path.c_str(), true);

  if (argc < 2) {
    return 0;
  }

  char* args[argc];
  memcpy(args, argv + 1, (argc - 1) * sizeof(char*));
  args[argc - 1] = NULL;

  return execvpe(args[0], args, environ);
}
