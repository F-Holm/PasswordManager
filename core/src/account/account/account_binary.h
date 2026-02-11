#pragma once

#include "crypto/crypto.h"

struct AccountBinary {
  // --- logical field sizes ---
  

  // --- layout ---
  static constexpr std::size_t kDataSize =
      kEmailSize + kUsernameSize + kPasswordSize + kExtraSize;

  char description[kDescriptionSize];
  char data[kDataSize];
  char iv_description[Crypto::Size::kIv];
  char iv_data[Crypto::Size::kIv];
  char tag_description[Crypto::Size::kTag];
  char tag_data[Crypto::Size::kTag];

  void SetZero() noexcept;
  ~AccountBinary() noexcept;
};
