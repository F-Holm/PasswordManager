#include "data_block.h"

using std::string;

DataBlock::DataBlock(const size_t largo, unsigned char *str)
    : largo(largo), str(new unsigned char[largo]) {
  std::memcpy(this->str, str, largo);
}

DataBlock::DataBlock(string str)
    : largo(str.size()), str(new unsigned char[largo]) {
  std::memcpy(this->str, reinterpret_cast<unsigned char *>(&str[0]),
              str.size());
}

DataBlock::DataBlock(const DataBlock &other)
    : largo(other.largo), str(new unsigned char[largo]) {
  std::memcpy(this->str, other.str, largo);
}

DataBlock::DataBlock() : largo(0), str(nullptr) {}

DataBlock::~DataBlock() { delete[] str; }

DataBlock &DataBlock::operator=(const DataBlock &other) {
  if (this != &other) {
    delete[] str;
    largo = other.largo;
    str = new unsigned char[largo];
    memcpy(this->str, other.str, largo);
  }
  return *this;
}