#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>

#include "db/db_status.h"
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

  DBStatus AddAccount(const AccountBinary& account);
  std::expected<AccountBinary, DBStatus> GetAccount(std::int64_t position);
  DBStatus UpdateAccount(std::int64_t position, const AccountBinary& account);
  DBStatus DeleteAccount(std::int64_t position);
  DBStatus MoveAccount(std::int64_t from_position, std::int64_t to_position);
  std::expected<std::vector<AccountIndex>, DBStatus> GetAllIndex();
  std::expected<std::size_t, DBStatus> Count();
  DBStatus Compact();

 private:
  sqlite3* db_ = nullptr;

  bool CreateSchema();
  bool Execute(std::string_view sql);
  // Returns the DBStatus matching the connection's last error.
  [[nodiscard]] DBStatus LastStatus() const noexcept;
};
