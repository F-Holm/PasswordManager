#pragma once

#include "types/account_binary.h"

#include <openssl/crypto.h>

void AccountBinary::SetZero() noexcept {
  explicit_bzero(this, sizeof(AccountBinary));
}

AccountBinary::~AccountBinary() noexcept { this->SetZero(); }
