#pragma once

#include <string>

struct fs_event {
  char event;
  bool isdir;
  std::string file;
};

typedef void (*inotify_cb)(fs_event event);

bool winevfs_init_listen(inotify_cb cb);
void winevfs_watch_directory(std::string directory);
