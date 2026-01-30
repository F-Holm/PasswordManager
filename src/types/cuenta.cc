#pragma once

#include "types/cuenta.h"

    Cuenta::~Cuenta() noexcept {
        desc            = nullptr;
        email_          = nullptr;
        nombre_usuario_ = nullptr;
        contra_         = nullptr;
        extra           = nullptr;
        tag_desc        = nullptr;
        tag_bloque      = nullptr;
        iv_desc         = nullptr;
        iv_bloque       = nullptr;
    }
