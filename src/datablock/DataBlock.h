#pragma once

#include <string>

struct DataBlock {
  size_t largo;
  unsigned char *str;

  DataBlock(const size_t largo, unsigned char *str);
  DataBlock(std::string str);
  DataBlock(const DataBlock &other);
  DataBlock();
  ~DataBlock();

  DataBlock &operator=(const DataBlock &other);
};