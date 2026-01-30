#pragma once

#include "types/account.h"

void Account::Bind(const AccountBinary& account_binary) noexcept{
    this->description = &account_binary.description;
    char->email       = &account_binary.data;
    char->user_name   = &account_binary.data + kEmailSize;
    char->password    = &account_binary.data + kEmailSize + kUsernameSize;
    char->extra       = &account_binary.data + kEmailSize + kUsernameSize + kPasswordSize;
}

void Account::Nullify() noexcept {
    description = nullptr;
    email       = nullptr;
    user_name   = nullptr;
    password    = nullptr;
    extra       = nullptr;
}

Account::~Account() noexcept {
    this->Nullify();
}
