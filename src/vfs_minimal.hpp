#pragma once

#ifndef AT_FDCWD
#  define AT_FDCWD -100
#endif

enum Intent
  {
   Intent_Read,
   Intent_Modify,
   Intent_Create,
   Intent_Delete
  };

const char* winevfs_get_path(const char* in, Intent intent, int atfd=AT_FDCWD);
