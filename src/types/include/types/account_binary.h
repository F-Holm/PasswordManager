#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "crypto/crypto.h"
#include "types/account_size.h"

struct AccountBinary {
  // Bumped by the database on every update (not encrypted, not secret); lets
  // an edit detect that someone else changed the account since it was read.
  std::int64_t revision = 0;
  std::array<std::byte, AccountSize::kUid> uid;
  std::array<std::byte, AccountSize::kDescription> description;
  std::array<std::byte, AccountSize::kData> data;
  std::array<std::byte, Crypto::Size::kIv> iv_description;
  std::array<std::byte, Crypto::Size::kIv> iv_data;
  std::array<std::byte, Crypto::Size::kTag> tag_description;
  std::array<std::byte, Crypto::Size::kTag> tag_data;

  void SetZero() noexcept;
  ~AccountBinary() noexcept;
};
