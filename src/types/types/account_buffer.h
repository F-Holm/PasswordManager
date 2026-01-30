#pragma once

#include <string>

struct AccountBuffer {
  std::string description;
  std::string email;
  std::string user_name;
  std::string password;
  std::string extra;

  void Nullify() noexcept;
  ~Account() noexcept;
};
