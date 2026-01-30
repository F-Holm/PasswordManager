#pragma once

#include <openssl/crypto.h>

#include "types/ind_cuenta.h"

    IndCuenta::~IndCuenta() noexcept {
        explicit_bzero(this, sizeof(IndCuenta));
    }
