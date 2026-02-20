#pragma once

#include <cstddef>
#include <fstream>
#include <span>
#include <string_view>

#include "crypto/crypto.h"

template <typename T>
class DB {
 public:
  std::fstream file;
  T element;

  bool ReadPos(std::size_t pos);
  bool ReadNext();

  bool UpdatePos(std::size_t pos);
  bool UpdateNext();

  bool DeletePos(std::size_t pos);
  bool DeleteNext();

  bool Add();

  bool IsOpen();

  DB(std::string_view file_name);
  ~DB();

 private:
  std::size_t count;

  void Open(std::string_view file_name);
  void Close();
  void Clear();
  bool SetFilePos(std::size_t pos);
  void UpdateCount();
  bool IsEOF();
};

template <typename T>
bool DB<T>::ReadPos(std::size_t pos) {
  if (!SetFilePos(pos)) return false;
  return ReadNext();
}

template <typename T>
bool DB<T>::ReadNext() {
  if (!file.read(reinterpret_cast<char*>(&element), sizeof(element))) {
    file.clear();
    return false;
  }
  return true;
}

template <typename T>
bool DB<T>::UpdatePos(std::size_t pos) {
  if (!SetFilePos(pos)) return false;
  return UpdateNext();
}

template <typename T>
bool DB<T>::UpdateNext() {
  if (IsEOF() ||
      !file.write(reinterpret_cast<const char*>(&element), sizeof(element))) {
    return false;
  }
  file.flush();
  return true;
}

template <typename T>
bool DB<T>::DeletePos(std::size_t pos) {
  if (!SetFilePos(pos)) return false;
  return DeleteNext();
}

template <typename T>
bool DB<T>::DeleteNext() {
  Clear();
  return UpdateNext();
}

template <typename T>
bool DB<T>::Add() {
  file.seekp(count * sizeof(element));
  if (file.write(reinterpret_cast<const char*>(&element), sizeof(T))) {
    count++;
    file.flush();
    return true;
  }
  file.clear();
  return false;
}

template <typename T>
bool DB<T>::IsOpen() {
  return file.is_open();
}

template <typename T>
DB<T>::DB(std::string_view file_name) {
  Open(file_name);
  if (IsOpen()) UpdateCount();
}

template <typename T>
DB<T>::~DB() {
  Close();
}

template <typename T>
void DB<T>::Open(std::string_view file_name) {
  file.open(file_name.data(), std::ios::binary | std::ios::in | std::ios::out);
}

template <typename T>
void DB<T>::Close() {
  if (file.is_open()) {
    file.close();
  }
}

template <typename T>
void DB<T>::Clear() {
  Crypto::SecureClear(std::as_writable_bytes(std::span{&element, 1}));
}

template <typename T>
bool DB<T>::SetFilePos(std::size_t pos) {
  file.seekg(pos * sizeof(element), std::ios::beg);
  if (file.fail()) {
    file.clear();
    return false;
  }
  return true;
}

template <typename T>
void DB<T>::UpdateCount() {
  std::streampos current_pos = file.tellg();
  file.seekg(0, std::ios::end);
  std::streamoff total_bytes = file.tellg();
  file.seekg(current_pos);

  count = total_bytes <= 0
              ? 0
              : static_cast<std::size_t>(total_bytes / sizeof(element));
}

template <typename T>
bool DB<T>::IsEOF() {
  std::size_t current_element =
      static_cast<std::size_t>(file.tellg()) / sizeof(element);
  return current_element >= count;
}
