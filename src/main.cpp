#include "vfs.hpp"
#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
  winevfs_init();
  winevfs_write_vfsfile("/tmp/.winevfs");
  setenv("WINEVFS_VFSFILE", "/tmp/.winevfs", true);

  char self_path[PATH_MAX];
  readlink("/proc/self/exe", self_path, PATH_MAX);

  std::filesystem::path parent_path = self_path;
  parent_path = parent_path.parent_path().parent_path() / "$LIB/libwinevfs_lib.so";
  setenv("LD_PRELOAD", parent_path.c_str(), true);

  if (argc < 2) {
    return 0;
  }

  char* args[argc];
  memcpy(args, argv + 1, argc * sizeof(char*));
  args[argc - 1] = NULL;

  return execvp(args[0], args);
}
