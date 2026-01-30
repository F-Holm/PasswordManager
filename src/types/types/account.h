#pragma once

#include "types/account_binary.h"

struct Account {
    char* description = nullptr;
    char* email       = nullptr;
    char* user_name   = nullptr;
    char* password    = nullptr;
    char* extra       = nullptr;

    void Bind(const AccountBinary& account_binary) noexcept;
    void Nullify() noexcept;
    ~Account() noexcept;
};
