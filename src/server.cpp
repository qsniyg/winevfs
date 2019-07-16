#include "server.hpp"
#include "functions.hpp"
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <mutex>

static std::string socket_path() {
  return "/tmp/.winevfs/ipc_socket";
}

static int create_socket(bool is_server) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (fd < 0) {
    perror("socket error");fflush(stderr);
    return fd;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;

  std::string socketpath = socket_path();

  strncpy(addr.sun_path, socketpath.c_str(), sizeof(addr.sun_path)-1);

  if (is_server) {
    winevfs__unlink(socketpath.c_str());

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
      perror("bind error");fflush(stderr);
      close(fd);
      return -1;
    }

    if (listen(fd, 5) == -1) {
      perror("listen error");fflush(stderr);
      close(fd);
      return -1;
    }
  } else {
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
      perror("connect error");fflush(stderr);
      close(fd);
      return -1;
    }
  }

  return fd;
}

static int read_buffer(int fd, void* buffer, size_t len) {
  size_t orig_len = len;

  while (true) {
    int rc = read(fd, buffer, len);
    if (rc == (ssize_t)len)
      return orig_len;

    if (rc <= 0)
      return rc;

    buffer = (char*)buffer + (size_t)rc;
    len -= rc;
  }
}

//#define SERVER_BUILD
#ifdef SERVER_BUILD
#include <unordered_set>

static std::string patch_file;
static std::mutex file_mutex;

static std::unordered_set<int> fds;
static std::unordered_set<pthread_t> threads;
static int thread_count;
static std::mutex set_mutex;

static void process_command(char command, void* data, int size) {
  printf("Processing command %c (%i)\n", command, size);fflush(stdout);
  puts((char*)data);fflush(stdout);
  if (command == 'R' || command == 'D') {
    std::lock_guard<std::mutex> lock(file_mutex);
    FILE* fp = (FILE*)winevfs__fopen(patch_file.c_str(), "a");
    if (!fp) {
      printf("Unable to open patch file: %s\n", patch_file.c_str());fflush(stdout);
      return;
    }

    fputc(command, fp);
    fputc('\n', fp);

    fputs((char*)data, fp);
    printf("Dest: %s\n", data);fflush(stdout);
    fputc('\n', fp);

    if (command == 'R') {
      fputs((char*)data + strlen((char*)data) + 1, fp);
      printf("Src: %s\n", (char*)data + strlen((char*)data) + 1);fflush(stdout);
      fputc('\n', fp);
    } else {
      fputc('\n', fp);
    }

    fflush(fp);
    fclose(fp);
  }
}

static void* server_client_thread(void* data) {
  int clientfd = *(int*)data;

  while (true) {
    char command;
    int size;
    void* data = nullptr;

    puts("Awaiting command");fflush(stdout);
    int rc = read(clientfd, &command, 1);
    if (rc != 1) {
      //perror("read (command)");
      break;
    }

    printf("Command: %c\n", command);fflush(stdout);

    if (false) {
      // Send early to avoid possible races
      std::lock_guard<std::mutex> lock(set_mutex);
      for (auto it = fds.begin(); it != fds.end(); it++) {
        if (*it == clientfd)
          continue;

        int wc = write(*it, &command, 1);
        if (wc != 1)
          continue;
      }
    }

    rc = read_buffer(clientfd, &size, sizeof(size));
    if (rc != sizeof(size)) {
      perror("read (size)");
      break;
    }

    if (size > 0) {
      data = malloc(size);
      rc = read_buffer(clientfd, data, size);
      if (rc != size) {
        perror("read (data)");
        break;
      }
    }

    {
      std::lock_guard<std::mutex> lock(set_mutex);
      for (auto it = fds.begin(); it != fds.end(); it++) {
        if (*it == clientfd)
          continue;

        int wc = write(*it, &command, 1);
        if (wc != 1)
          continue;

        wc = write(*it, &size, sizeof(size));
        if (wc != sizeof(size))
          continue;

        if (size > 0) {
          wc = write(*it, data, size);
          if (wc != size)
            continue;
        }
      }
    }

    process_command(command, data, size);

    free(data);

    puts("Finished processing");fflush(stdout);
  }

  // end
  std::lock_guard<std::mutex> lock(set_mutex);
  fds.erase(clientfd);
  thread_count--;

  if (thread_count <= 0) {
    winevfs__unlink(socket_path().c_str());
    exit(0);
  }

  return NULL;
}

bool winevfs_init_server(char* patchfile) {
  patch_file = patchfile;

  int fd = create_socket(true);

  if (fd < 0) {
    puts("Error starting server");fflush(stdout);
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(set_mutex);
    threads.clear();
    fds.clear();
  }

  puts("Listening");fflush(stdout);

  bool accepted = false;
  while (true) {
    if (accepted && thread_count <= 0)
      break;

    int cl = accept(fd, NULL, NULL);

    // FIXME: should this kill the server?
    if (cl < 0) {
      perror("accept error");
      break;
    }

    accepted = true;

    {
      std::lock_guard<std::mutex> lock(set_mutex);
      fds.insert(cl);
      thread_count++;

      pthread_t thread;
      if (pthread_create(&thread, NULL, server_client_thread, &cl)) {
        perror("Unable to create thread");fflush(stderr);
        break;
      }

      threads.insert(thread);
    }
  }

  {
    std::lock_guard<std::mutex> lock(set_mutex);
    for (auto it = threads.begin(); it != threads.end(); it++) {
      pthread_join(*it, NULL);
    }
  };

  winevfs__unlink(socket_path().c_str());
  return true;
}
#endif

#ifndef SERVER_BUILD
static int serverfd = 0;
static pthread_t client_thread;

std::mutex winevfs_client_processing_mutex;
static void* client_thread_fn(void* thread_data) {
  command_cb cb = (command_cb)thread_data;
  while (true) {
    unsigned char command;
    int rc = read(serverfd, &command, 1);
    if (rc != 1) {
      perror("c: read (command)");
      break;
    }

    int size;
    rc = read_buffer(serverfd, &size, sizeof(size));
    if (rc != sizeof(size)) {
      perror("c: read (size)");
      break;
    }

    void* data = NULL;
    if (size > 0) {
      data = (char*)malloc(size);
      rc = read_buffer(serverfd, data, size);
      if (rc != size) {
        perror("c: read (data)");
        break;
      }
    }

    printf("Received command %c (%i)\n", command, size);fflush(stdout);

    {
      std::lock_guard<std::mutex> lock(winevfs_client_processing_mutex);
      cb(command, data, size);
    }

    free(data);

    printf("c: Finished processing command %c (%i)\n", command, size);fflush(stdout);
  }

  return NULL;
}

bool winevfs_init_client(command_cb cb) {
  int fd = create_socket(false);

  if (fd < 0)
    return false;

  serverfd = fd;
  if (pthread_create(&client_thread, NULL, client_thread_fn, (void*)cb)) {
    perror("Unable to create client thread");fflush(stderr);
    return false;
  }

  return true;
}

void winevfs_send_command(char command, void* data, int size) {
  printf("Sending command: %c (%i)\n", command, size);fflush(stdout);
  if (serverfd <= 0)
    return;

  write(serverfd, &command, 1);

  write(serverfd, &size, sizeof(size));
  write(serverfd, data, size);
}
#endif
