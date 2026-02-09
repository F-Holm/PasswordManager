#include "secure_clear.h"

#include <openssl/crypto.h>

void SecureClear::SetZero(void* data, size_t size) noexcept {
  explicit_bzero(data, size);
}
