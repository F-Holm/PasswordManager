#pragma once

#include <string>

struct DataBlock {
  size_t largo;
  char *str;

  DataBlock(size_t largo, char *str);
  explicit DataBlock(const std::string &str);
  DataBlock(const DataBlock &other);
  DataBlock();
  ~DataBlock();

  auto operator=(const DataBlock &other) -> DataBlock &;
};