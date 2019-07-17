#pragma once

#include "vfs_minimal.hpp"
#include "vfs_types.hpp"
#include <filesystem>
#include <string>

std::string winevfs_lower(std::string& in);
std::filesystem::path winevfs_abspath(std::filesystem::path source, int atfd=AT_FDCWD);
void winevfs_add_read_directory(std::filesystem::path source, std::filesystem::path destination,
                                std::string search="");
void winevfs_add_read_file(std::filesystem::path source, std::filesystem::path destination);
void winevfs_add_write_directory(std::filesystem::path source, std::filesystem::path destination);
void winevfs_init(bool client = false);
void winevfs_write_vfsfile(char* envfile);
std::string winevfs_get_path(std::filesystem::path in, Intent intent, int atfd=AT_FDCWD);
