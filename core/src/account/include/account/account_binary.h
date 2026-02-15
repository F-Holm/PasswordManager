#pragma once

#include "account/account_size.h"
#include "crypto/crypto.h"

struct AccountBinary {
  std::byte description[AccountSize::kDescription];
  std::byte data[AccountSize::kData];
  std::byte iv_description[Crypto::Size::kIv];
  std::byte iv_data[Crypto::Size::kIv];
  std::byte tag_description[Crypto::Size::kTag];
  std::byte tag_data[Crypto::Size::kTag];

  void SetZero() noexcept;
  ~AccountBinary() noexcept;
};
