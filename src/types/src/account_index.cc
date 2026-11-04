#include "types/account_index.h"

#include <cstring>
#include <span>

#include "crypto/crypto.h"

AccountIndex::AccountIndex() noexcept
    : position(-1), uid(), description(), iv_description(), tag_description() {
  this->SetZero();
}

AccountIndex::AccountIndex(std::int64_t position) noexcept
    : position(position),
      uid(),
      description(),
      iv_description(),
      tag_description() {
  this->SetZero();
}

AccountIndex::AccountIndex(AccountBinary& account) noexcept
    : position(-1),
      uid(account.uid),
      description(account.description),
      iv_description(account.iv_description),
      tag_description(account.tag_description) {
}

AccountIndex::AccountIndex(AccountBinary& account,
                           std::int64_t position) noexcept
    : position(position),
      uid(account.uid),
      description(account.description),
      iv_description(account.iv_description),
      tag_description(account.tag_description) {
}

void AccountIndex::SetZero() noexcept {
  Crypto::SecureClear(std::span{uid});
  Crypto::SecureClear(std::span{description});
  Crypto::SecureClear(std::span{iv_description});
  Crypto::SecureClear(std::span{tag_description});
}

AccountIndex::~AccountIndex() noexcept {
  Crypto::SecureClear(
      std::span{reinterpret_cast<std::byte*>(this), sizeof(*this)});
}
