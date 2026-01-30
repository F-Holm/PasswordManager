#pragma once

#include <openssl/crypto.h>

struct IndCuenta {
    char desc[128];

    ~IndCuenta() noexcept;
};
