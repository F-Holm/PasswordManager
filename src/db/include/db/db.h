#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>

#include "db/db_status.h"
#include "types/account_binary.h"
#include "types/account_index.h"
#include "types/vault_meta.h"

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

  enum class OpenMode : std::uint8_t {
    // Create the database file if it does not exist.
    kCreateIfMissing,
    // Fail if the database file does not exist.
    kExistingOnly,
  };

  bool Open(std::string_view path, OpenMode mode = OpenMode::kCreateIfMissing);
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

  // Vault metadata (a single row). GetMeta() returns kNotFound for a vault
  // that has not been initialized yet; CreateMeta() returns kAlreadyExists
  // if it has; UpdateMeta() returns kNotFound if it has not.
  std::expected<VaultMeta, DBStatus> GetMeta();
  DBStatus CreateMeta(const VaultMeta& meta);
  DBStatus UpdateMeta(const VaultMeta& meta);

 private:
  sqlite3* db_ = nullptr;

  bool CreateSchema();
  DBStatus WriteMeta(std::string_view sql, const VaultMeta& meta);
  bool Execute(std::string_view sql);
  // Returns the DBStatus matching the connection's last error.
  [[nodiscard]] DBStatus LastStatus() const noexcept;
};
