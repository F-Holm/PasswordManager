#pragma once

struct Cuenta {
    char* desc            = nullptr;
    char* email_          = nullptr;
    char* nombre_usuario_ = nullptr;
    char* contra_         = nullptr;
    char* extra           = nullptr;
    char* tag_desc        = nullptr;
    char* tag_bloque      = nullptr;
    char* iv_desc         = nullptr;
    char* iv_bloque       = nullptr;

    ~Cuenta() noexcept;
};
