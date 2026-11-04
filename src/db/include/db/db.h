#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <string_view>
#include <utility>
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

  // Accounts are addressed by their `id`: a stable identifier assigned on
  // insertion, never reused and unaffected by removing or reordering other
  // accounts (unlike their position in the list).

  // Appends `account` at the end of the list. Writes its id to *out_id if
  // given.
  DBStatus AddAccount(const AccountBinary& account,
                      std::int64_t* out_id = nullptr);
  std::expected<AccountBinary, DBStatus> GetAccount(std::int64_t id);
  // Passed as `expected_revision` to overwrite regardless of the revision.
  static constexpr std::int64_t kAnyRevision = -1;

  // Overwrites the account `id` and bumps its revision, provided it is still
  // at `expected_revision` (kConflict otherwise; skip the check with
  // kAnyRevision). Its uid never changes: kNotFound unless the stored
  // account `id` has `account.uid`.
  DBStatus UpdateAccount(std::int64_t id, std::int64_t expected_revision,
                         const AccountBinary& account);
  // Deletes the account `id`, shifting the positions after it down.
  DBStatus DeleteAccount(std::int64_t id);
  // Moves the account `id` to `to_position`, shifting the accounts in
  // between. kNotFound if there is no such account or position.
  DBStatus MoveAccount(std::int64_t id, std::int64_t to_position);
  std::expected<std::vector<AccountIndex>, DBStatus> GetAllIndex();
  // Every account with its id, ordered by position.
  std::expected<std::vector<std::pair<std::int64_t, AccountBinary>>, DBStatus>
  GetAllAccounts();
  std::expected<std::size_t, DBStatus> Count();
  DBStatus Compact();

  // A number that changes whenever *another* connection (e.g. another
  // program using the same vault) commits a change to the database; this
  // connection's own commits leave it unchanged. Only comparisons between
  // two values read on the same open connection are meaningful.
  std::expected<std::int64_t, DBStatus> DataVersion();

  // Runs `body` inside one write transaction: either everything it does is
  // committed, or (if it returns anything but kOk, or the commit fails)
  // nothing is. DB methods called from `body` join the transaction.
  DBStatus RunInTransaction(const std::function<DBStatus()>& body);

  // Vault metadata (a single row). GetMeta() returns kNotFound for a vault
  // that has not been initialized yet; CreateMeta() returns kAlreadyExists
  // if it has; UpdateMeta() returns kNotFound if it has not.
  std::expected<VaultMeta, DBStatus> GetMeta();
  DBStatus CreateMeta(const VaultMeta& meta);
  DBStatus UpdateMeta(const VaultMeta& meta);

 private:
  sqlite3* db_ = nullptr;

  bool CreateSchema();
  // Position of the account `id`, or kNotFound.
  std::expected<std::int64_t, DBStatus> PositionOf(std::int64_t id);
  DBStatus WriteMeta(std::string_view sql, const VaultMeta& meta);
  bool Execute(std::string_view sql);
  // Returns the DBStatus matching the connection's last error.
  [[nodiscard]] DBStatus LastStatus() const noexcept;
};
