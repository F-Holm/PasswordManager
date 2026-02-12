#pragma once

#include <array>
#include <cstddef>

#include "account/account_binary.h"
#include "account/account_size.h"

struct AccountIndex {
  std::array<std::byte, AccountSize::kDescription> description;

  AccountIndex(AccountBinary& account) noexcept;
  void SetZero() noexcept;
  ~AccountIndex() noexcept;
};
