#pragma once

#include <openssl/crypto.h>

#include "types/cuenta_arch.h"

CuentaArch::~CuentaArch() noexcept {
    explicit_bzero(this, sizeof(CuentaArch));
}
