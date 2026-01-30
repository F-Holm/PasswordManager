#pragma once

#include "types/account_binary.h"

struct AccountIndex {
  char description[kDescriptionSize];

  void SetZero() noexcept;
  ~AccountIndex() noexcept;
};
