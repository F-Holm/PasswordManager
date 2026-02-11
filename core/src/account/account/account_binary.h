#pragma once

#include "account/account_size.h"
#include "crypto/crypto.h"

struct AccountBinary {
  char description[AccountSize::kDescription];
  char data[AccountSize::kDataSize];
  char iv_description[Crypto::Size::kIv];
  char iv_data[Crypto::Size::kIv];
  char tag_description[Crypto::Size::kTag];
  char tag_data[Crypto::Size::kTag];

  void SetZero() noexcept;
  ~AccountBinary() noexcept;
};
