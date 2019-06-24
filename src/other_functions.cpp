#include <stdio.h>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
// Dirty hack, but how else are we going to do it?
#define _DIRENT_H
#include <dirent.h>
#include <bits/types.h>
#include <bits/dirent.h>
#include <dlfcn.h>
#include "vfs_types.hpp"
#include "functions.hpp"

//extern char** environ;

struct DIR;

extern std::unordered_map<std::string, unique_vector> winevfs_folder_mappings;
extern std::mutex winevfs_folder_mappings_mutex;
std::string winevfs_abspath(std::string source);

struct opendir_base_info {
  std::string path;
  bool finished;
  size_t position;
};

struct opendir_info {
  struct opendir_base_info info;
  struct dirent* temp;
};

struct opendir_info64 {
  struct opendir_base_info info;
  struct dirent64* temp;
};

static std::unordered_map<DIR*, opendir_info> opendir_mappings;
static std::unordered_map<DIR*, opendir_info64> opendir64_mappings;
static std::mutex opendir_mappings_mutex;
std::string winevfs_lower(std::string& in);

static bool winevfs_opendir_fill_info(char* path, opendir_base_info* info) {
  std::string string_path = winevfs_abspath(std::string(path));
  info->path = winevfs_lower(string_path);

  info->finished = false;
  info->position = 0;

  std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
  auto it = winevfs_folder_mappings.find(info->path);
  if (it == winevfs_folder_mappings.end()) {
    return false;
  }

  return true;
}

extern "C" {
  void winevfs_add_opendir(DIR* dir, char* path) {
    struct opendir_info info;
    if (!winevfs_opendir_fill_info(path, &info.info))
      return;

    info.temp = new dirent;

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    opendir_mappings[dir] = info;
  }

  void winevfs_add_opendir64(DIR* dir, char* path) {
    struct opendir_info64 info;
    if (!winevfs_opendir_fill_info(path, &info.info))
      return;

    info.temp = new dirent64;

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    opendir64_mappings[dir] = info;
  }
}

static bool get_filename(opendir_base_info* info, std::string* filename) {
  info->finished = true;

  std::lock_guard<std::mutex> folder_lock(winevfs_folder_mappings_mutex);

  unique_vector* uv = &winevfs_folder_mappings[info->path];
  if (info->position >= uv->vector.size())
    return false;

  *filename = uv->vector[info->position++];
  return true;
}

//void winevfs_do_closedir(

extern "C" {
  /*extern int execve(char const *path, char* const *argv, char* const *envp);
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
    }*/
  struct dirent* readdir(DIR* dirp) {
    //puts("readdir");
    struct dirent* entry = (struct dirent*)winevfs__readdir((void*)dirp);
    if (entry != NULL) {
      return entry;
    }

    std::string filename;

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    auto it = opendir_mappings.find(dirp);
    if (it == opendir_mappings.end()) {
      return NULL;
    }

    if (!get_filename(&it->second.info, &filename))
      return NULL;

    memcpy(it->second.temp->d_name, filename.c_str(), filename.size() + 1);

    return it->second.temp;
  }

  struct dirent64* readdir64(DIR* dirp) {
    //puts("readdir64");
    struct dirent64* entry = (struct dirent64*)winevfs__readdir64((void*)dirp);
    if (entry != NULL) {
      return entry;
    }

    std::string filename;

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    auto it = opendir64_mappings.find(dirp);
    if (it == opendir64_mappings.end()) {
      return NULL;
    }

    if (!get_filename(&it->second.info, &filename))
      return NULL;

    memcpy(it->second.temp->d_name, filename.c_str(), filename.size() + 1);

    return it->second.temp;
  }

  int closedir(DIR* dirp) {
    //puts("closedir");
    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);

    auto it = opendir_mappings.find(dirp);
    if (it != opendir_mappings.end()) {
      delete it->second.temp;
      opendir_mappings.erase(it);
    }

    auto it64 = opendir64_mappings.find(dirp);
    if (it64 != opendir64_mappings.end()) {
      delete it64->second.temp;
      opendir64_mappings.erase(it64);
    }

    return winevfs__closedir((void*)dirp);
  }
}
