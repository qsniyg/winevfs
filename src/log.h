#pragma once

#define WINEVFS_LOG_LEVEL_NONE 0
#define WINEVFS_LOG_LEVEL_INFO 1
#define WINEVFS_LOG_LEVEL_TRACE 2

extern int winevfs_log_level;
void winevfs_log(int level, const char* str, ...);

#define trace(...) winevfs_log(WINEVFS_LOG_LEVEL_TRACE, __VA_ARGS__)
