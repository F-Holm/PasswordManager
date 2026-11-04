#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "types/account_binary.h"
#include "types/account_index.h"

struct sqlite3;

class DB {
 public:
  // How long a write waits for another connection (e.g. a second program
  // using the same vault) to release the database lock before giving up.
  static constexpr int kDefaultBusyTimeoutMs = 5000;

  DB() noexcept;
  ~DB();

  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;
  DB(DB&&) = delete;
  DB& operator=(DB&&) = delete;

  bool Open(std::string_view path);
  void Close();
  [[nodiscard]] bool IsOpen() const noexcept;
  // Overrides the busy timeout of the open connection. Returns false if the
  // database is not open or `milliseconds` is negative.
  bool SetBusyTimeout(int milliseconds);

  bool AddAccount(const AccountBinary& account);
  std::optional<AccountBinary> GetAccount(std::int64_t position);
  bool UpdateAccount(std::int64_t position, const AccountBinary& account);
  bool DeleteAccount(std::int64_t position);
  bool MoveAccount(std::int64_t from_position, std::int64_t to_position);
  std::vector<AccountIndex> GetAllIndex();
  std::size_t Count();
  bool Compact();

 private:
  sqlite3* db_ = nullptr;

  bool CreateSchema();
  bool Execute(std::string_view sql);
};
