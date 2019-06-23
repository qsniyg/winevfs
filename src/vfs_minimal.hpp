#pragma once

enum Intent
  {
   Intent_Read,
   Intent_Modify,
   Intent_Create,
   Intent_Delete
  };

const char* winevfs_get_path(const char* in, Intent intent);
