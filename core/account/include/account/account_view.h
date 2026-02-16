#pragma once

#include <cstddef>
#include <span>

#include "account/account_binary.h"

struct AccountView {
  std::span<std::byte> description;
  std::span<std::byte> email;
  std::span<std::byte> user_name;
  std::span<std::byte> password;
  std::span<std::byte> extra;

  AccountView(AccountBinary& account) noexcept;
  void Clear() noexcept;
  ~AccountView() noexcept;
};
