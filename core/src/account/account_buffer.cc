#include "types/account_buffer.h"

static void NullifyStr(std::string& s) {
  std::fill(s.begin(), s.end(), 0);
  s.clear();
}

void Account::Nullify() noexcept {
  NullifyStr(description);
  NullifyStr(email);
  NullifyStr(user_name);
  NullifyStr(password);
  NullifyStr(extra);
}

Account::~Account() noexcept { this->Nullify(); }
