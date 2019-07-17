#pragma once

#include <unordered_set>
#include <vector>

struct unique_vector {
  std::unordered_set<std::string> set;
  std::vector<std::string> vector;

  void insert(std::string string);
  void erase(std::string string);
  bool empty();
  std::string last();
};

struct folder_mapping {
  //std::string folderpath;
  unique_vector folder_paths;

  unique_vector children;
};
