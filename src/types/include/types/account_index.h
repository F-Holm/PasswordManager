#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "crypto/crypto.h"
#include "types/account_binary.h"
#include "types/account_size.h"

struct AccountIndex {
  // Stable identifier of the account (never reused, unaffected by reordering).
  std::int64_t id = -1;
  // Place of the account in the user's list; changes when accounts are
  // removed or reordered.
  std::int64_t position;
  std::int64_t revision = 0;
  std::array<std::byte, AccountSize::kUid> uid;
  std::array<std::byte, AccountSize::kDescription> description;
  std::array<std::byte, Crypto::Size::kIv> iv_description;
  std::array<std::byte, Crypto::Size::kTag> tag_description;

  AccountIndex() noexcept;
  AccountIndex(std::int64_t position) noexcept;
  AccountIndex(AccountBinary& account) noexcept;
  AccountIndex(AccountBinary& account, std::int64_t position) noexcept;
  void SetZero() noexcept;
  ~AccountIndex() noexcept;
};
