#pragma once

#include "account/account_size.h"

struct AccountIndex {
  char description[AccountSize::kDescription];

  void SetZero() noexcept;
  ~AccountIndex() noexcept;
};
