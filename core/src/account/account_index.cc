#pragma once

#include "account/account_index.h"

#include "crypto/crypto.h"

#include <cstddef>
#include <span>

void AccountIndex::SetZero() noexcept {
  SecureClear(std::as_writable_bytes(std::span{description}));
}

AccountIndex::~AccountIndex() noexcept { this->SetZero(); }
