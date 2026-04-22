#pragma once

#include <cstdint>

#include "db/storage.h"
#include "types/account_binary.h"
#include "types/account_index.h"
#include "types/account_size.h"

class DB {
 public:
  void Open();
  void Close();

  AccountIndex SwapIndex(std::uint32_t elem1, std::uint32_t elem2);
  AccountIndex GetAllIndex();
  void AddAccount(const AccountBinary& account);
  AccountBinary GetAccount(std::uint32_t pos);
  void UpdateAccount(std::uint32_t pos, const AccountBinary& account);
  void DeleteAccount(std::uint32_t pos);
  void Compact();

  DB();
  ~DB();

 private:
  Storage<AccountBinary> vault;
  Storage<AccountIndex> account_index;
};
