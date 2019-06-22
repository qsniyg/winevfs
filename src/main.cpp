#include "vfs.hpp"
#include <iostream>

int main(int argc, char** argv) {
  std::cout << winevfs_get_path(std::filesystem::path(argv[1]), Intent_Read) << std::endl;
  return 0;
}
