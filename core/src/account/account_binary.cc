#pragma once

#include "account/account_binary.h"

#include "crypto/crypto.h"

#include <span>
#include <stddef>

void AccountBinary::SetZero() noexcept {
  SecureClear(std::as_writable_bytes(std::span{description}));
  SecureClear(std::as_writable_bytes(std::span{data}));
  SecureClear(std::as_writable_bytes(std::span{iv_description}));
  SecureClear(std::as_writable_bytes(std::span{iv_data}));
  SecureClear(std::as_writable_bytes(std::span{tag_description}));
  SecureClear(std::as_writable_bytes(std::span{tag_data}));
}

AccountBinary::~AccountBinary() noexcept { this->SetZero(); }
