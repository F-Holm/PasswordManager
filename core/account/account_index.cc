#include "account/account_index.h"

#include <cstring>
#include <span>

#include "crypto/crypto.h"

AccountIndex::AccountIndex(AccountBinary& account) noexcept
    : description(account.description) {}

void AccountIndex::SetZero() noexcept {
  Crypto::SecureClear(std::span{description});
}

AccountIndex::~AccountIndex() noexcept { this->SetZero(); }
