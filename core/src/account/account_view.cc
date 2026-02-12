#include "account/account_view.h"

#include "account/account_size.h"

AccountView::AccountView(AccountBinary& account) noexcept
    : description(reinterpret_cast<std::byte*>(account.description),
                  AccountSize::kDescription),
      email(reinterpret_cast<std::byte*>(account.data), AccountSize::kEmail),
      user_name(
          reinterpret_cast<std::byte*>(account.data) + AccountSize::kEmail,
          AccountSize::kUsername),
      password(reinterpret_cast<std::byte*>(account.data) +
                   AccountSize::kEmail + AccountSize::kUsername,
               AccountSize::kPassword),
      extra(reinterpret_cast<std::byte*>(account.data) + AccountSize::kEmail +
                AccountSize::kUsername + AccountSize::kPassword,
            AccountSize::kExtra) {}

void AccountView::Clear() noexcept {
  description = {};
  email = {};
  user_name = {};
  password = {};
  extra = {};
}

AccountView::~AccountView() noexcept { this->Clear(); }
