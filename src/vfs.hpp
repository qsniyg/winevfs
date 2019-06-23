#pragma once

#include "vfs_minimal.hpp"
#include <filesystem>
#include <string>

std::filesystem::path winevfs_abspath(std::filesystem::path source);
void winevfs_add_read_directory(std::filesystem::path source, std::filesystem::path destination);
void winevfs_add_read_file(std::filesystem::path source, std::filesystem::path destination);
void winevfs_add_write_directory(std::filesystem::path source, std::filesystem::path destination);
void winevfs_init();
void winevfs_write_vfsfile(char* envfile);
std::string winevfs_get_path(std::filesystem::path in, Intent intent);
