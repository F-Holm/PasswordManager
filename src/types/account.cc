#pragma once

#include "types/account.h"

Account::Account(const AccountBinary& account_binary) noexcept
    : description(account_binary.description, AccountBinary::kDescriptionSize),
      email(std::string_view(account_binary.data, AccountBinary::kEmailSize)),
      user_name(
          std::string_view(account_binary.data + AccountBinary::kEmailSize,
                           AccountBinary::kUsernameSize)),
      password(std::string_view(account_binary.data +
                                    AccountBinary::kEmailSize +
                                    AccountBinary::kUsernameSize,
                                AccountBinary::kPasswordSize)),
      extra(std::string_view(account_binary.data + AccountBinary::kEmailSize +
                                 AccountBinary::kUsernameSize +
                                 AccountBinary::kPasswordSize,
                             AccountBinary::kExtraSize)) {}
