#include "log.h"
#include <string>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdarg.h>
#include <mutex>
#include <string.h>

//#define _GNU_SOURCE
#include <stdio.h>

int winevfs_log_level = WINEVFS_LOG_LEVEL_NONE;

static bool loglevel_inited = false;
static std::mutex loglevel_inited_mutex;

static void init_loglevel() {
  std::lock_guard<std::mutex> lock(loglevel_inited_mutex);
  if (loglevel_inited)
    return;

  char* loglevel = getenv("WINEVFS_LOGLEVEL");
  if (loglevel && loglevel[0]) {
    if (!strcmp(loglevel, "info")) {
      winevfs_log_level = WINEVFS_LOG_LEVEL_INFO;
    } else if (!strcmp(loglevel, "trace")) {
      winevfs_log_level = WINEVFS_LOG_LEVEL_TRACE;
    } else if (!strcmp(loglevel, "debug")) {
      winevfs_log_level = WINEVFS_LOG_LEVEL_DEBUG;
    }
  }

  loglevel_inited = true;
}

void winevfs_log(int level, const char* str, ...) {
  init_loglevel();
  if (level > winevfs_log_level)
    return;

  const char* levelstr = " INFO";
  if (level == WINEVFS_LOG_LEVEL_TRACE)
    levelstr = "TRACE";
  else if (level == WINEVFS_LOG_LEVEL_DEBUG)
    levelstr = "DEBUG";

  char header[128];

  pid_t tid = syscall(SYS_gettid);
  sprintf(header, "[%s:(%i/%i)] ", levelstr, getpid(), tid);

  char* out;

  va_list list;
  va_start(list, str);
  vasprintf(&out, str, list);
  va_end(list);

  printf("%s%s\n", header, out);fflush(stdout);

  free(out);
}
