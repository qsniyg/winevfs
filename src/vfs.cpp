#include "vfs.hpp"
#include <unordered_map>
#include <string>
#include <algorithm>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mutex>
#include <string.h>
#include <iostream>

static std::string lower(std::string& in) {
  std::string out = in;
  std::transform(out.begin(), out.end(), out.begin(), ::tolower);
  return out;
}

static int fs_stat(std::filesystem::path path, struct stat* buf) {
  std::string path_str = path;
  return stat(path_str.c_str(), buf);
}

static bool fs_exists(std::filesystem::path source) {
  struct stat s;
  return fs_stat(source, &s) == 0;
}

static bool fs_isdir(std::filesystem::path source) {
  struct stat s;
  fs_stat(source, &s);

  return S_ISDIR(s.st_mode);
}

static DIR* fs_readdir(std::filesystem::path source) {
  std::string path_str = source;
  return opendir(path_str.c_str());
}

std::filesystem::path fs_getcwd() {
  char path[PATH_MAX];
  getcwd(path, PATH_MAX);

  return std::filesystem::path(path);
}

static std::filesystem::path abspath_simple(std::filesystem::path source) {
  std::filesystem::path new_path;

  for (auto it = source.begin(); it != source.end(); it++) {
    if (*it == ".")
      continue;
    if (*it == "..") {
      new_path = new_path.parent_path();
      continue;
    }

    new_path /= *it;
  }

  return new_path;
}

static std::filesystem::path abspath(std::filesystem::path source) {
  if (source.is_absolute())
    return source;

  std::filesystem::path path = abspath_simple(source);
  // TODO: replace, maybe cache?
  path = fs_getcwd() / path;
  return abspath_simple(path);
  //return std::filesystem::absolute(source);
}

std::unordered_map<std::string, std::string> read_mappings;
std::mutex read_mappings_mutex;
std::unordered_map<std::string, std::string> write_mappings;
std::mutex write_mappings_mutex;
//std::unordered_map<std::string, std::string> unlink_mappings; // TODO

void winevfs_add_read_directory(std::filesystem::path source, std::filesystem::path destination) {
  source = abspath(source);
  destination = abspath(destination);

  for (const auto& entry : std::filesystem::directory_iterator(source)) {
    std::string out = destination / entry.path().filename();
    std::string path = entry.path();

    if (entry.is_directory()) {
      winevfs_add_read_directory(path, out);
    } else {
      read_mappings[lower(path)] = out;
    }
  }
}

void winevfs_add_read_file(std::filesystem::path source, std::filesystem::path destination) {
  source = abspath(source);
  destination = abspath(destination);
  std::string source_string = source;
  read_mappings[lower(source_string)] = destination;
}

void winevfs_add_write_directory(std::filesystem::path source, std::filesystem::path destination) {
  source = abspath(source);
  destination = abspath(destination);
  std::string source_string = source;
  write_mappings[lower(source_string)] = destination;
}

std::unordered_map<std::string, std::filesystem::path> path_cache;
static std::filesystem::path winpath(std::filesystem::path source) {
  if (fs_exists(source)) {
    return source;
  }

  source = abspath(source);

  std::string path_str = source;
  path_str = lower(path_str);

  auto it = path_cache.find(path_str);
  if (it != path_cache.end())
    return it->second;

  if (!source.has_parent_path())
    return source;

  std::filesystem::path parent = source.parent_path();
  std::filesystem::path basename = source.filename();
  std::string basename_lower = basename;
  basename_lower = lower(basename_lower);

  std::filesystem::path winparent = winpath(parent);
  std::string winparent_lower = winparent;
  winparent_lower = lower(winparent_lower);

  it = path_cache.find(winparent_lower);
  if (it == path_cache.end())
    path_cache[winparent_lower] = winparent;

  if (fs_isdir(winparent)) {
    DIR* d = fs_readdir(winparent);
    struct dirent* entry;

    if (d) {
      while ((entry = readdir(d)) != NULL) {
        std::string filename = entry->d_name;
        if (lower(filename) == basename_lower) {
          return winparent / filename;
        }
      }
    }
  }

  return winparent / basename;
}

std::string winevfs_get_path(std::filesystem::path in, Intent intent) {
  std::filesystem::path path = winpath(abspath(in));
  std::string path_lower = path;
  path_lower = lower(path_lower);

  {
    std::lock_guard<std::mutex> lock(read_mappings_mutex);
    auto it = read_mappings.find(path_lower);
    if (it != read_mappings.end()) {
      return it->second;
    }
  };

  if (intent == Intent_Modify || intent == Intent_Create) {
    std::lock_guard<std::mutex> lock(write_mappings_mutex);

    for (auto it = write_mappings.begin(); it != write_mappings.end(); it++) {
      if (!strncmp(path_lower.c_str(), it->first.c_str(), it->first.size())) {
        return std::filesystem::path(it->second) / std::filesystem::path(path_lower.c_str() + it->first.size());
      }
    }
  }

  return path;
}
