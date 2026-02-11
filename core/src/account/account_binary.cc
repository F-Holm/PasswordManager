#include "account/account_binary.h"

#include <cstddef>
#include <span>

void AccountBinary::SetZero() noexcept {
  Crypto::SecureClear(std::as_writable_bytes(std::span{description}));
  Crypto::SecureClear(std::as_writable_bytes(std::span{data}));
  Crypto::SecureClear(std::as_writable_bytes(std::span{iv_description}));
  Crypto::SecureClear(std::as_writable_bytes(std::span{iv_data}));
  Crypto::SecureClear(std::as_writable_bytes(std::span{tag_description}));
  Crypto::SecureClear(std::as_writable_bytes(std::span{tag_data}));
}

AccountBinary::~AccountBinary() noexcept { this->SetZero(); }
