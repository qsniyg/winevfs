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
//#include <dirent.h>
#include <features.h>
#include <bits/types.h>
#include <bits/types/struct_iovec.h>
#include <bits/dirent.h>
#define _SYS_SOCKET_H
#include <bits/socket.h>
#define _FCNTL_H
#define _SYS_STAT_H
#define _SYS_STATFS_H
#include <bits/stat.h>
#include <bits/statfs.h>
#include <dlfcn.h>
#include "vfs_types.hpp"
#include "vfs_minimal.hpp"
#include "functions.hpp"
#include "log.h"

//extern char** environ;

struct DIR {
  int fd;
  // a bunch of other stuff as well
};

extern std::unordered_map<std::string, folder_mapping> winevfs_folder_mappings;
extern std::unordered_map<std::string, std::string> winevfs_reverse_folder_mappings;
extern std::mutex winevfs_folder_mappings_mutex;
std::string winevfs_abspath(std::string source, int atfd=AT_FDCWD);
std::string winevfs_winpath(std::string source, int atfd);
std::string winevfs_get_fd_path(int atfd);
void winevfs_setcwd(const char* cwd);
int winevfs_stat(std::string path_str, struct stat64* buf, bool use_cache = false);
void winevfs_init(bool client, bool force);

extern std::unordered_map<int, std::string> winevfs_fd_table;
extern std::mutex winevfs_fd_table_mutex;

struct opendir_base_info {
  std::string path;
  bool finished;
  size_t position;
  std::unordered_set<std::string> already;
};

struct opendir_info {
  struct opendir_base_info info;
  struct dirent* temp;
  struct dirent64* temp64;
};

struct opendir_info64 {
  struct opendir_base_info info;
  struct dirent64* temp;
};

static std::unordered_map<DIR*, opendir_info> opendir_mappings;
static std::unordered_map<DIR*, opendir_info64> opendir64_mappings;
static std::mutex opendir_mappings_mutex;
std::string winevfs_lower(std::string& in);

static bool winevfs_opendir_fill_info(char* path, int atfd, opendir_base_info* info) {
  std::string string_path = winevfs_abspath(std::string(path), atfd);
  info->path = winevfs_lower(string_path);

  //puts(info->path.c_str());

  info->finished = false;
  info->position = 0;

  std::lock_guard<std::mutex> lock(winevfs_folder_mappings_mutex);
  auto it = winevfs_folder_mappings.find(info->path);
  if (it == winevfs_folder_mappings.end()) {
    info->path = winevfs_reverse_folder_mappings[info->path];
    //puts(info->path.c_str());

    it = winevfs_folder_mappings.find(info->path);
    if (it == winevfs_folder_mappings.end()) {
      return false;
    }
  }

  return true;
}

static void add_path_to_fdtable(int fd, std::string string_path, int atfd) {
  if (fd <= 0)
    return;

  std::lock_guard<std::mutex> lock(winevfs_fd_table_mutex);
  winevfs_fd_table[fd] = string_path;
  //printf("open wrap: %s (%i)\n", string_path.c_str(), fd);fflush(stdout);
}

extern "C" {
  void winevfs_wrap_open(int fd, char* path, int atfd) {
    std::string string_path = winevfs_abspath(std::string(path), atfd);
    add_path_to_fdtable(fd, string_path, atfd);
  }

  void winevfs_add_opendir(DIR* dir, char* path, int atfd) {
    if (!dir) {
      return;
    }

    struct opendir_info info;
    winevfs_opendir_fill_info(path, atfd, &info.info);
    if (!winevfs_opendir_fill_info(path, atfd, &info.info))
      return;

    info.temp = new dirent;
    info.temp64 = new dirent64;

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    opendir_mappings[dir] = info;

    //add_path_to_fdtable(dir->fd, info.info.path, atfd);
  }

  void winevfs_add_opendir64(DIR* dir, char* path, int atfd) {
    return winevfs_add_opendir(dir, path, atfd);

    struct opendir_info64 info;
    if (!winevfs_opendir_fill_info(path, atfd, &info.info))
      return;

    info.temp = new dirent64;

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    opendir64_mappings[dir] = info;
  }
}

static bool get_filename(opendir_base_info* info, std::string* filename) {
  info->finished = true;

  std::lock_guard<std::mutex> folder_lock(winevfs_folder_mappings_mutex);

  folder_mapping* uv = &winevfs_folder_mappings[info->path];
  if (info->position >= uv->children.vector.size())
    return false;

  *filename = uv->children.vector[info->position++];
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
    //trace("readdir(dirp=%p)", dirp);
    struct dirent* entry = (struct dirent*)winevfs__readdir((void*)dirp);

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    auto it = opendir_mappings.find(dirp);

    if (entry != NULL) {
      if (it != opendir_mappings.end()) {
        std::string name = entry->d_name;
        it->second.info.already.insert(winevfs_lower(name));
      }

      //trace("readdir: %s", entry->d_name);
      debug("readdir(dirp=%p): %s", dirp, entry->d_name);
      //puts(entry->d_name);fflush(stdout);

      return entry;
    }

    if (it == opendir_mappings.end()) {
      return NULL;
    }

    std::string filename;

    while (true) {
      if (!get_filename(&it->second.info, &filename))
        return NULL;

      if (it->second.info.already.find(winevfs_lower(filename)) != it->second.info.already.end())
        continue;

      strcpy(it->second.temp->d_name, filename.c_str());
      //trace("readdir (vfs): %s", it->second.temp->d_name);
      debug("readdir(dirp=%p) vfs: %s", dirp, it->second.temp->d_name);
      //puts(it->second.temp->d_name);fflush(stdout);

      return it->second.temp;
    }
  }

  struct dirent64* readdir64(DIR* dirp) {
    //puts("readdir64");
    //trace("readdir64(dirp=%p)", dirp);
    if (sizeof(dirent64) == sizeof(dirent)) {
      return (struct dirent64*)readdir(dirp);
    }

    struct dirent64* entry = (struct dirent64*)winevfs__readdir64((void*)dirp);

    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);
    auto it = opendir_mappings.find(dirp);

    if (entry != NULL) {
      if (it != opendir_mappings.end()) {
        std::string name = entry->d_name;
        it->second.info.already.insert(winevfs_lower(name));
      }

      debug("readdir64(dirp=%p): %s", dirp, entry->d_name);
      //puts(entry->d_name);fflush(stdout);

      return entry;
    }

    if (it == opendir_mappings.end()) {
      return NULL;
    }

    std::string filename;

    while (true) {
      if (!get_filename(&it->second.info, &filename))
        return NULL;

      if (it->second.info.already.find(winevfs_lower(filename)) != it->second.info.already.end())
        continue;

      strcpy(it->second.temp64->d_name, filename.c_str());
      debug("readdir64(dirp=%p) vfs: %s", dirp, it->second.temp64->d_name);
      //puts(it->second.temp64->d_name);fflush(stdout);

      return it->second.temp64;
    }
  }

  int closedir(DIR* dirp) {
    //puts("closedir");
    std::lock_guard<std::mutex> lock(opendir_mappings_mutex);

    auto it = opendir_mappings.find(dirp);
    if (it != opendir_mappings.end()) {
      //printf("closedir: %s\n", it->second.info.path.c_str());fflush(stdout);
      delete it->second.temp;
      delete it->second.temp64;
      opendir_mappings.erase(it);
    } else {
      //puts("closedir out of mappings");fflush(stdout);
    }

    auto it64 = opendir64_mappings.find(dirp);
    if (it64 != opendir64_mappings.end()) {
      delete it64->second.temp;
      opendir64_mappings.erase(it64);
    }

    if (false && dirp) {
      std::lock_guard<std::mutex> fd_lock(winevfs_fd_table_mutex);
      winevfs_fd_table.erase(dirp->fd);
    }

    return winevfs__closedir((void*)dirp);
  }

  int close(int fd) {
    //printf("close %i\n", fd);fflush(stdout);
    std::lock_guard<std::mutex> lock(winevfs_fd_table_mutex);
    if (false) {
      auto it = winevfs_fd_table.find(fd);
      if (it != winevfs_fd_table.end()) {
        //printf("close: %s\n", it->second.c_str());fflush(stdout);
      } else {
        //puts("close out of mappings");fflush(stdout);
      }
    }
    winevfs_fd_table.erase(fd);

    return winevfs__close(fd);
  }

  int _dup(int fd) {
    //puts("dup");fflush(stdout);
    int newfd = winevfs__dup(fd);
    //printf("dup: %i -> %i\n", fd, newfd);fflush(stdout);
    std::lock_guard<std::mutex> lock(winevfs_fd_table_mutex);
    auto it = winevfs_fd_table.find(fd);
    if (it != winevfs_fd_table.end())
      winevfs_fd_table[newfd] = it->second;

    return newfd;
  }

  // For debugging wineserver
  int dup2(int fd1, int fd2) {
    if (fd2 == 0 || fd2 == 1)
      return fd2;

    return winevfs__dup2(fd1, fd2);
  }

  int __xstat64(int ver, const char* path, struct stat64* buf) {
    trace(" in: __xstat64(path=%s)", path);
    //puts("__xstat64");fflush(stdout);
    //puts(path);fflush(stdout);
    path = winevfs_get_path(path, Intent_Read, AT_FDCWD);
    //trace("out: __xstat64(path=%s)", path);
    //puts(path);fflush(stdout);
    int ret = winevfs_stat(path, buf, false);
    trace("ret: __xstat64(path=%s): %i", path, ret);
    //printf("ret: %i\n", ret);fflush(stdout);
    free((void*)path);
    return ret;
  }

  int stat64(const char* path, struct stat64* buf) {
    return __xstat64(3, path, buf);
  }

  int __xstat(int ver, const char* path, struct stat* buf) {
    //puts("__xstat");fflush(stdout);
    if (sizeof(struct stat) == sizeof (struct stat64)) {
      return __xstat64(ver, path, (struct stat64*)buf);
    }

    trace(" in: __xstat(path=%s)", path);

    //puts(path);fflush(stdout);
    path = winevfs_get_path(path, Intent_Read, AT_FDCWD);
    //puts(path);fflush(stdout);
    int ret = winevfs____xstat(ver, path, buf);
    trace("ret: __xstat(path=%s): %i", path, ret);
    free((void*)path);
    return ret;
  }

  int stat(const char* path, struct stat* buf) {
    // FIXME?
    return __xstat(3, path, buf);
  }

  int __statfs(const char* file, struct statfs* buf) {
    //puts("__statfs");fflush(stdout);
    //puts(file);fflush(stdout);
    file = winevfs_get_path(file, Intent_Read, AT_FDCWD);
    //puts(file);fflush(stdout);
    int ret = winevfs____statfs(file, buf);
    free((void*)file);
    //printf("ret: %i\n", ret);fflush(stdout);
    buf->f_type = 0x65735546;
    return ret;
  }

  int statfs(const char* file, struct statfs* buf) {
    //puts("statfs");fflush(stdout);
    //puts(file);fflush(stdout);
    file = winevfs_get_path(file, Intent_Read, AT_FDCWD);
    //puts(file);fflush(stdout);
    int ret = winevfs__statfs(file, buf);
    free((void*)file);
    //printf("ret: %i\n", ret);fflush(stdout);

    buf->f_type = 0x65735546;
    return ret;
  }

  int statfs64(const char* file, struct statfs64* buf) {
    //puts("statfs64");fflush(stdout);
    //puts(file);fflush(stdout);
    file = winevfs_get_path(file, Intent_Read, AT_FDCWD);
    //puts(file);fflush(stdout);
    int ret = winevfs__statfs64(file, (struct statfs*)buf);
    free((void*)file);
    //printf("ret: %i\n", ret);fflush(stdout);

    buf->f_type = 0x65735546;
    return ret;
  }

  int fstatfs1(int fd, struct statfs* buf) {
    //puts("fstatfs");fflush(stdout);
    int ret = winevfs__fstatfs(fd, buf);

    //printf("ret: %i\n", ret);fflush(stdout);
    buf->f_type = 0x65735546;
    return ret;
  }

  // This should be done in fork() instead, but
  //   it would make it twice as slow to fork() + execve()
  //   as it would be run twice
  // This function is called in wineserver
  pid_t setsid() {
    //puts("setsid");fflush(stdout);
    winevfs_init(true, true);
    return winevfs__setsid();
  }
}
