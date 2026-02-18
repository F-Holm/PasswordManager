#pragma once

#include <array>
#include <cstddef>

#include "account/account_size.h"
#include "crypto/crypto.h"

struct AccountBinary {
  std::array<std::byte, AccountSize::kDescription> description;
  std::array<std::byte, AccountSize::kData> data;
  std::array<std::byte, Crypto::Size::kIv> iv_description;
  std::array<std::byte, Crypto::Size::kIv> iv_data;
  std::array<std::byte, Crypto::Size::kTag> tag_description;
  std::array<std::byte, Crypto::Size::kTag> tag_data;

  void SetZero() noexcept;
  ~AccountBinary() noexcept;
};
