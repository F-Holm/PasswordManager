#include "account/account_index.h"

#include <cstring>
#include <span>

#include "crypto/crypto.h"

AccountIndex::AccountIndex(AccountBinary& account) noexcept {
  std::memcpy(description.data(), account.description, description.size());
}

void AccountIndex::SetZero() noexcept {
  Crypto::SecureClear(std::span{description});
}

AccountIndex::~AccountIndex() noexcept { this->SetZero(); }
