#pragma once

#include "types/account_index.h"

#include <openssl/crypto.h>

void AccountIndex::SetZero() noexcept {
  explicit_bzero(this, sizeof(AccountIndex));
}

AccountIndex::~AccountIndex() noexcept { this->SetZero(); }
