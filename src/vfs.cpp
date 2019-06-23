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
#include "functions.hpp"

static std::string lower(std::string& in) {
  std::string out = in;
  std::transform(out.begin(), out.end(), out.begin(), ::tolower);
  return out;
}

static int real_stat(const char* str, struct stat* buf) {
  return winevfs____xstat(_STAT_VER_LINUX, str, buf);
}

static int fs_stat(std::filesystem::path path, struct stat* buf) {
  std::string path_str = path;
  return real_stat(path_str.c_str(), buf);
}

static bool fs_exists(std::filesystem::path source) {
  struct stat s;
  return fs_stat(source, &s) == 0;
}

static bool fs_isdir(std::filesystem::path source) {
  struct stat s;
  if (fs_stat(source, &s))
    return false;

  return S_ISDIR(s.st_mode);
}

static DIR* fs_opendir(std::filesystem::path source) {
  std::string path_str = source;
  return (DIR*)winevfs__opendir(path_str.c_str());
}

void fs_mkdir_p(std::filesystem::path source) {
  if (!source.has_parent_path()) {
    return;
  }

  if (!fs_isdir(source)) {
    std::filesystem::path parent = source.parent_path();
    fs_mkdir_p(parent);

    winevfs__mkdir(std::string(source).c_str(), 0777);
  }
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
      if (new_path.has_parent_path()) {
        new_path = new_path.parent_path();
        continue;
      }
    }

    new_path /= *it;
  }

  return new_path;
}

static std::filesystem::path abspath(std::filesystem::path source) {
  std::filesystem::path path = abspath_simple(source);

  if (path.is_absolute())
    return path;

  // TODO: maybe cache?
  path = fs_getcwd() / path;
  return abspath_simple(path);
  //return std::filesystem::absolute(source);
}

// TODO: somehow make this work better multi-process
static std::unordered_map<std::string, std::string> read_mappings;
static std::mutex read_mappings_mutex;
static std::unordered_map<std::string, std::string> write_mappings;
static std::mutex write_mappings_mutex;
//std::unordered_map<std::string, std::string> unlink_mappings; // TODO
static bool inited = false;
static std::mutex inited_mutex;

void _add_read_entry(std::string source, std::string destination) {
  std::lock_guard<std::mutex> lock(read_mappings_mutex);
  read_mappings[lower(source)] = destination;
  //std::cout << source << std::endl;
  //std::cout << destination << std::endl;
}

void winevfs_add_read_directory(std::filesystem::path source, std::filesystem::path destination) {
  source = abspath(source);
  destination = abspath(destination);

  if (fs_isdir(destination)) {
    DIR* d = fs_opendir(destination);

    if (d) {
      struct dirent* entry;
      while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') {
          if (entry->d_name[1] == 0)
            continue;

          if (entry->d_name[1] == '.' && entry->d_name[2] == 0)
            continue;
        }

        std::string out = destination / entry->d_name;
        std::string path = source / entry->d_name;

        if (fs_isdir(out)) {
          winevfs_add_read_directory(path, out);
        } else {
          _add_read_entry(path, out);
        }
      }

      return;
    }
  }

  _add_read_entry(source, destination);
}

void winevfs_add_read_file(std::filesystem::path source, std::filesystem::path destination) {
  source = abspath(source);
  destination = abspath(destination);

  _add_read_entry(source, destination);
}

void winevfs_add_write_directory(std::filesystem::path source, std::filesystem::path destination) {
  source = abspath(source);
  destination = abspath(destination);
  std::string source_string = source;

  std::lock_guard<std::mutex> lock(write_mappings_mutex);
  write_mappings[lower(source_string)] = destination;
}

static std::unordered_map<std::string, std::filesystem::path> path_cache;
static std::mutex path_cache_mutex;
static std::filesystem::path winpath(std::filesystem::path source) {
  if (fs_exists(source)) {
    return source;
  }

  source = abspath(source);

  std::string path_str = source;
  path_str = lower(path_str);

  {
    std::lock_guard<std::mutex> lock(path_cache_mutex);
    auto it = path_cache.find(path_str);
    if (it != path_cache.end())
      return it->second;
  }

  if (!source.has_parent_path())
    return source;

  std::filesystem::path parent = source.parent_path();
  std::filesystem::path basename = source.filename();
  std::string basename_lower = basename;
  basename_lower = lower(basename_lower);

  std::filesystem::path winparent = winpath(parent);
  std::string winparent_lower = winparent;
  winparent_lower = lower(winparent_lower);

  {
    std::lock_guard<std::mutex> lock(path_cache_mutex);
    auto it = path_cache.find(winparent_lower);
    if (it == path_cache.end())
      path_cache[winparent_lower] = winparent;
  }

  if (fs_isdir(winparent)) {
    DIR* d = fs_opendir(winparent);
    struct dirent* entry;

    if (d) {
      while ((entry = readdir(d)) != NULL) {
        std::string filename = entry->d_name;
        if (lower(filename) == basename_lower) {
          closedir(d);
          return winparent / filename;
        }
      }

      closedir(d);
    }
  }

  return winparent / basename;
}

void winevfs_read_vfsfile(char* envfile) {
  FILE* fp = (FILE*)winevfs__fopen(envfile, "r");

  ssize_t read;
  size_t len = 0;
  char* line = NULL;

  bool quick = false;
  bool first = true;

  int phase = 0;
  bool is_read = false;
  std::string input = "";
  std::string output = "";

  while ((read = getline(&line, &len, fp)) != -1) {
    // Remove trailing \n
    line[strlen(line) - 1] = 0;

    if (first) {
      first = false;

      if (!strcmp(line, "quick")) {
        quick = true;
        continue;
      }
    }

    if (phase == 0) {
      if (line[0] == 'R') {
        is_read = true;
      } else if (line[0] == 'W') {
        is_read = false;
      }

      phase = 1;
      continue;
    }

    if (phase == 1) {
      input = line;

      phase = 2;
      continue;
    }

    if (phase == 2) {
      output = line;

      if (is_read) {
        if (quick) {
          winevfs_add_read_file(input, output);
        } else {
          winevfs_add_read_directory(input, output);
        }
      } else {
        winevfs_add_write_directory(input, output);
      }

      phase = 0;
      continue;
    }
  }

  fclose(fp);
}

void winevfs_init() {
  std::lock_guard<std::mutex> lock(inited_mutex);
  if (inited)
    return;

  char* envfile = getenv("WINEVFS_VFSFILE");
  if (!envfile || !envfile[0])
    return;

  winevfs_read_vfsfile(envfile);
  inited = true;
}

std::string winevfs_get_path(std::filesystem::path in, Intent intent) {
  winevfs_init();

  //std::cout << in << std::endl;
  std::filesystem::path path = winpath(abspath(in));
  //std::cout << path << std::endl;
  std::string path_lower = path;
  path_lower = lower(path_lower);

  // TODO: Handle Intent_Delete

  {
    std::lock_guard<std::mutex> lock(read_mappings_mutex);
    auto it = read_mappings.find(path_lower);
    if (it != read_mappings.end()) {
      return it->second;
    }
  };

  if (intent == Intent_Modify || intent == Intent_Create) {
    {
      std::lock_guard<std::mutex> lock(write_mappings_mutex);

      for (auto it = write_mappings.begin(); it != write_mappings.end(); it++) {
        if (!strncmp(path_lower.c_str(), it->first.c_str(), it->first.size())) {
          const char* rest = path.c_str() + it->first.size();
          if (rest[0] == '/')
            rest++;

          std::filesystem::path newpath = std::filesystem::path(it->second) / std::filesystem::path(rest);
          fs_mkdir_p(newpath.parent_path());

          std::lock_guard<std::mutex> read_lock(read_mappings_mutex);
          read_mappings[path_lower] = newpath;
          //std::cout << it->second << std::endl;
          //std::cout << path_lower << std::endl;
          //std::cout << newpath << std::endl;
          return newpath;
        }
      }
    };
  }

  return path;
}

const char* winevfs_get_path(const char* in, Intent intent) {
  std::string retval = winevfs_get_path(std::filesystem::path(in), intent);
  char* ret;
  if (true) {
    ret = (char*)malloc(retval.size() + 1);
    strcpy(ret, retval.c_str());
  } else {
    ret = (char*)malloc(strlen(in) + 1);
    strcpy(ret, in);
  }
  return ret;
}
