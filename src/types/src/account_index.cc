#include "types/account_index.h"

#include <cstring>
#include <span>

#include "crypto/crypto.h"

AccountIndex::AccountIndex() noexcept : description(), position(-1) {
  this->SetZero();
}

AccountIndex::AccountIndex(std::int64_t position) noexcept
    : description(), position(position) {
  this->SetZero();
}

AccountIndex::AccountIndex(AccountBinary& account) noexcept
    : description(account.description), position(-1) {
}

AccountIndex::AccountIndex(AccountBinary& account,
                           std::int64_t position) noexcept
    : description(account.description), position(position) {
}

void AccountIndex::SetZero() noexcept {
  Crypto::SecureClear(std::span{description});
}

AccountIndex::~AccountIndex() noexcept {
  this->SetZero();
  position = 0;
}
