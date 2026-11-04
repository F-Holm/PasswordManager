#include "types/account_binary.h"

#include <cstddef>
#include <span>

void AccountBinary::SetZero() noexcept {
  revision = 0;
  Crypto::SecureClear(uid);
  Crypto::SecureClear(description);
  Crypto::SecureClear(data);
  Crypto::SecureClear(iv_description);
  Crypto::SecureClear(iv_data);
  Crypto::SecureClear(tag_description);
  Crypto::SecureClear(tag_data);
}

AccountBinary::~AccountBinary() noexcept {
  this->SetZero();
}
