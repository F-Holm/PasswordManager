#include "account/account_view.h"

#include "account/account_size.h"

AccountView::AccountView(AccountBinary& account) noexcept
    : description(account.description),
      email(std::span(account.data).first(AccountSize::kEmail)),
      user_name(std::span(account.data)
                    .subspan(AccountSize::kEmail, AccountSize::kUsername)),
      password(std::span(account.data)
                   .subspan(AccountSize::kEmail + AccountSize::kUsername,
                            AccountSize::kPassword)),
      extra(std::span(account.data)
                .subspan(AccountSize::kEmail + AccountSize::kUsername +
                             AccountSize::kPassword,
                         AccountSize::kExtra)) {}

void AccountView::Clear() noexcept {
  description = {};
  email = {};
  user_name = {};
  password = {};
  extra = {};
}

AccountView::~AccountView() noexcept { this->Clear(); }
