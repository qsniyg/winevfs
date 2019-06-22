#pragma once

#include <filesystem>
#include <string>

enum Intent
  {
   Intent_Read,
   Intent_Modify,
   Intent_Create
  };

void winevfs_add_read_directory(std::filesystem::path source, std::filesystem::path destination);
void winevfs_add_read_file(std::filesystem::path source, std::filesystem::path destination);
void winevfs_add_write_directory(std::filesystem::path source, std::filesystem::path destination);
std::string winevfs_get_path(std::filesystem::path in, Intent intent);
