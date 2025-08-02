#include "data_block.hh"

#include <cstring>

DataBlock::DataBlock(const size_t largo, char *str) : largo(largo), str(new char[largo]) { std::memcpy(this->str, str, largo); }

DataBlock::DataBlock(const std::string &str) : largo(str.size()), str(new char[largo]) { std::memcpy(this->str, (&str[0]), str.size()); }

DataBlock::DataBlock(const DataBlock &other) : largo(other.largo), str(new char[largo]) { std::memcpy(this->str, other.str, largo); }

DataBlock::DataBlock() : largo(0), str(nullptr) {}

DataBlock::~DataBlock() { delete[] str; }

auto DataBlock::operator=(const DataBlock &other) -> DataBlock & {
  if (this != &other) {
    largo = other.largo;
    delete[] str;
    str = new char[largo];
    memcpy(this->str, other.str, largo);
  }
  return *this;
}