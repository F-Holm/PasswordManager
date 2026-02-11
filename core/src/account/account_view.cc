#include "account/account_view.h"

void AccountView::Clear() noexcept {
  description = {};
  email = {};
  user_name = {};
  password = {};
  extra = {};
}

AccountView::~AccountView() noexcept { this->Clear(); }
