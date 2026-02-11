#include "account/account_index.h"

#include <cstddef>
#include <span>

#include "crypto/crypto.h"

void AccountIndex::SetZero() noexcept {
  Crypto::SecureClear(std::as_writable_bytes(std::span{description}));
}

AccountIndex::~AccountIndex() noexcept { this->SetZero(); }
