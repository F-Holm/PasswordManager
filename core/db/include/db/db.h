#pragma once

#include <cstddef>
#include <span>
#include <string_view>

template<typename T>
class DB {
  public:
    const std::string_view file_name;
    T element;

    std::span<T> ReadAll();
    bool ReadPos(std::size_t pos);
    bool ReadNext();

    DB();
    ~DB();
  private:

  void Open();
  void Close();
};