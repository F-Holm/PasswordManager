#pragma once

#include <string_view>

#include "types/account_binary.h"

struct Account {
  std::string_view description;
  std::string_view email;
  std::string_view user_name;
  std::string_view password;
  std::string_view extra;

  Account(const AccountBinary& account_binary) noexcept;
  void Nullify() noexcept;
  ~Account() noexcept;
};
