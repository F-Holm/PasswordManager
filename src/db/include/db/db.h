#pragma once

#include "types/account_binary.h"
#include "types/account_index.h"
#include "types/account_size.h"
#include "db/storage.h"

class DB {
 public:
  void Open
 private:
  Storage<AccountBinary> vault;
  Storage<AccountIndex> account_index;
};
