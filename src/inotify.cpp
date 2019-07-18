#include "inotify.hpp"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <unordered_map>
#include <string>
#include <string.h>
#include <mutex>
#include <unistd.h>

static std::unordered_map<int, std::string> fdtable;
static std::unordered_map<std::string, int> reverse_fdtable;
static std::mutex fdtable_mutex;
static int inotify_fd = -1;
static inotify_cb fs_cb;
static pthread_t listener_thread;

static void process_inotify_event(struct inotify_event* ievent) {
  //puts("Processing inotify event");fflush(stdout);

  struct fs_event event;

  if (ievent->mask & IN_CREATE ||
      ievent->mask & IN_MOVED_TO) {
    event.event = 'R';
  }

  if (ievent->mask & IN_DELETE ||
      ievent->mask & IN_DELETE_SELF ||
      ievent->mask & IN_MOVED_FROM) {
    event.event = 'D';
  }

  event.isdir = ievent->mask & IN_ISDIR;

  {
    std::lock_guard<std::mutex> lock(fdtable_mutex);
    event.file = fdtable[ievent->wd];
  }

  if (!(ievent->mask & IN_DELETE_SELF)) {
    event.file += "/" + std::string(ievent->name);
  } // TODO: else: remove from fdtable

  fs_cb(event);
}

static void read_inotify_events() {
  char buf[4096]
    __attribute__ ((aligned(__alignof__(struct inotify_event))));
  size_t buf_off = 0;
  struct inotify_event* event;

  while (true) {
    int rc = read(inotify_fd, buf + buf_off, 4096 - buf_off);
    if (rc == -1 && errno != EAGAIN) {
      perror("(inotify) read");
      return;
    }

    if (rc <= 0)
      break;

    buf_off = 0;

    int buffer_i = 0;
    while (buffer_i < rc) {
      event = (struct inotify_event*)&buf[buffer_i];
      int event_size = sizeof(struct inotify_event) + event->len;

      if (buffer_i + event_size > rc) {
        memmove(buf + buffer_i, buf, rc - buffer_i);
        buf_off = buffer_i;
        break;
      }

      process_inotify_event(event);

      buffer_i += event_size;
    }
  }
}

static void* listener_thread_fn(void*) {
  struct pollfd fds[1];

  fds[0].fd = inotify_fd;
  fds[0].events = POLLIN;

  while (true) {
    int poll_num = poll(fds, 1, -1);
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;

      perror("poll");
      break;
    }

    if (poll_num > 0) {
      if (fds[0].revents & POLLIN) {
        read_inotify_events();
      }
    }
  }

  return nullptr;
}

// Fork kills this
bool winevfs_init_listen(inotify_cb _cb) {
  //puts("Initializing inotify");fflush(stdout);

  fs_cb = _cb;
  inotify_fd = inotify_init1(IN_NONBLOCK);
  if (inotify_fd == -1) {
    perror("inotify_init1");
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(fdtable_mutex);
    fdtable.clear();
    reverse_fdtable.clear();
  }

  if (pthread_create(&listener_thread, NULL, listener_thread_fn, NULL)) {
    perror("pthread_create");fflush(stderr);
    return false;
  }

  return true;
}

void winevfs_watch_directory(std::string directory) {
  if (inotify_fd < 0) {
    puts("Error adding directory: inotify_fd < 0");fflush(stdout);
    return;
  }

  //printf("Adding %s to watched directories\n", directory.c_str());fflush(stdout);

  std::lock_guard<std::mutex> lock(fdtable_mutex);

  auto it = reverse_fdtable.find(directory);
  if (it != reverse_fdtable.end())
    return;

  int fd = inotify_add_watch(inotify_fd, directory.c_str(),
                             IN_DELETE |
                             IN_DELETE_SELF |
                             IN_CREATE |
                             IN_MOVED_FROM |
                             IN_MOVED_TO);

  fdtable[fd] = directory;
  reverse_fdtable[directory] = fd;
}
