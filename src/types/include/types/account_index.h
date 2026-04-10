#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "types/account_binary.h"
#include "types/account_size.h"

struct AccountIndex {
  std::int64_t position;
  std::array<std::byte, AccountSize::kDescription> description;

  AccountIndex() noexcept;
  AccountIndex(std::int64_t position) noexcept;
  AccountIndex(AccountBinary& account) noexcept;
  AccountIndex(AccountBinary& account, std::int64_t position) noexcept;
  void SetZero() noexcept;
  ~AccountIndex() noexcept;
};
