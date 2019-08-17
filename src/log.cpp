#include "log.h"
#include <string>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdarg.h>

//#define _GNU_SOURCE
#include <stdio.h>

int winevfs_log_level = WINEVFS_LOG_LEVEL_TRACE;

void winevfs_log(int level, const char* str, ...) {
  if (level > winevfs_log_level)
    return;

  const char* levelstr = " INFO";
  if (winevfs_log_level == WINEVFS_LOG_LEVEL_TRACE)
    levelstr = "TRACE";
  else if (winevfs_log_level == WINEVFS_LOG_LEVEL_DEBUG)
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
