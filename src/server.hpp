#pragma once

#include <stddef.h>

typedef void (*command_cb)(char command, void* data, int size);
bool winevfs_init_client(command_cb cb);
void winevfs_send_command(char command, void* data, int size);

#ifdef SERVER_BUILD
bool winevfs_init_server(char* path);
#endif
