#pragma once

#include <unordered_set>
#include <vector>

struct unique_vector {
  std::unordered_set<std::string> set;
  std::vector<std::string> vector;

  void insert(std::string string);
};
