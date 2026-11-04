#include "db/db.h"

#include <sqlite3.h>

#include <cstdint>
#include <string>
#include <utility>

#include "db/statement.h"
#include "db/transaction.h"

DB::DB() noexcept = default;

DB::~DB() {
  Close();
}

bool DB::Open(std::string_view path, OpenMode mode) {
  Close();

  const std::string path_str(path);
  int flags = SQLITE_OPEN_READWRITE;
  if (mode == OpenMode::kCreateIfMissing) {
    flags |= SQLITE_OPEN_CREATE;
  }
  if (sqlite3_open_v2(path_str.c_str(), &db_, flags, nullptr) != SQLITE_OK) {
    Close();
    return false;
  }

  sqlite3_busy_timeout(db_, kDefaultBusyTimeoutMs);
  Execute("PRAGMA foreign_keys = ON;");
  Execute("PRAGMA journal_mode = WAL;");

  if (!CreateSchema()) {
    Close();
    return false;
  }

  return true;
}

void DB::Close() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

bool DB::IsOpen() const noexcept {
  return db_ != nullptr;
}

bool DB::SetBusyTimeout(int milliseconds) {
  if (!IsOpen() || milliseconds < 0) {
    return false;
  }
  return sqlite3_busy_timeout(db_, milliseconds) == SQLITE_OK;
}

DBStatus DB::LastStatus() const noexcept {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }
  const DBStatus status = DBStatusFromSqlite(sqlite3_errcode(db_));
  // A failed step always has a non-OK error code; never report success for
  // an operation that is being aborted.
  return status == DBStatus::kOk ? DBStatus::kError : status;
}

bool DB::Execute(std::string_view sql) {
  const std::string sql_str(sql);
  return sqlite3_exec(db_, sql_str.c_str(), nullptr, nullptr, nullptr) ==
         SQLITE_OK;
}

bool DB::CreateSchema() {
  return Execute(
      "CREATE TABLE IF NOT EXISTS accounts ("
      "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  uid BLOB NOT NULL UNIQUE,"
      "  position INTEGER NOT NULL,"
      "  revision INTEGER NOT NULL DEFAULT 0,"
      "  description BLOB NOT NULL,"
      "  data BLOB NOT NULL,"
      "  iv_description BLOB NOT NULL,"
      "  iv_data BLOB NOT NULL,"
      "  tag_description BLOB NOT NULL,"
      "  tag_data BLOB NOT NULL"
      ");"
      "CREATE INDEX IF NOT EXISTS idx_accounts_position "
      "  ON accounts(position);"
      "CREATE TABLE IF NOT EXISTS vault_meta ("
      "  singleton INTEGER PRIMARY KEY CHECK (singleton = 0),"
      "  version INTEGER NOT NULL,"
      "  kdf_salt BLOB NOT NULL,"
      "  kdf_memory_kib INTEGER NOT NULL,"
      "  kdf_iterations INTEGER NOT NULL,"
      "  kdf_parallelism INTEGER NOT NULL,"
      "  verifier BLOB NOT NULL,"
      "  verifier_iv BLOB NOT NULL,"
      "  verifier_tag BLOB NOT NULL,"
      "  key_epoch INTEGER NOT NULL"
      ");");
}

DBStatus DB::AddAccount(const AccountBinary& account, std::int64_t* out_id) {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return DBStatusFromSqlite(tx.BeginResult());
  }

  Statement insert(db_,
                   "INSERT INTO accounts (position, uid, description, data, "
                   "iv_description, iv_data, tag_description, tag_data) "
                   "VALUES ("
                   "  (SELECT COALESCE(MAX(position), -1) + 1 "
                   "     FROM accounts),"
                   "  ?, ?, ?, ?, ?, ?, ?);");
  if (!insert.IsValid()) {
    return LastStatus();
  }

  insert.BindBlob(1, account.uid);
  insert.BindBlob(2, account.description);
  insert.BindBlob(3, account.data);
  insert.BindBlob(4, account.iv_description);
  insert.BindBlob(5, account.iv_data);
  insert.BindBlob(6, account.tag_description);
  insert.BindBlob(7, account.tag_data);

  if (insert.Step() != SQLITE_DONE) {
    return LastStatus();
  }
  const std::int64_t id = sqlite3_last_insert_rowid(db_);

  if (!tx.Commit()) {
    return LastStatus();
  }
  if (out_id != nullptr) {
    *out_id = id;
  }
  return DBStatus::kOk;
}

std::expected<AccountBinary, DBStatus> DB::GetAccount(std::int64_t id) {
  if (!IsOpen()) {
    return std::unexpected(DBStatus::kClosed);
  }

  Statement select(db_,
                   "SELECT uid, description, data, iv_description, iv_data, "
                   "tag_description, tag_data, revision FROM accounts "
                   "WHERE id = ?;");
  if (!select.IsValid()) {
    return std::unexpected(LastStatus());
  }

  select.BindInt64(1, id);

  const int step = select.Step();
  if (step == SQLITE_DONE) {
    return std::unexpected(DBStatus::kNotFound);
  }
  if (step != SQLITE_ROW) {
    return std::unexpected(LastStatus());
  }

  AccountBinary account;
  const bool ok = select.ColumnBlob(0, account.uid) &&
                  select.ColumnBlob(1, account.description) &&
                  select.ColumnBlob(2, account.data) &&
                  select.ColumnBlob(3, account.iv_description) &&
                  select.ColumnBlob(4, account.iv_data) &&
                  select.ColumnBlob(5, account.tag_description) &&
                  select.ColumnBlob(6, account.tag_data);
  if (!ok) {
    return std::unexpected(DBStatus::kError);
  }
  account.revision = select.ColumnInt64(7);

  return account;
}

DBStatus DB::UpdateAccount(std::int64_t id, std::int64_t expected_revision,
                           const AccountBinary& account) {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }

  // The update and the conflict check must see the same state.
  Transaction tx(db_);
  if (!tx.IsActive()) {
    return DBStatusFromSqlite(tx.BeginResult());
  }

  {
    Statement update(
        db_,
        "UPDATE accounts SET description = ?, data = ?, iv_description = ?, "
        "iv_data = ?, tag_description = ?, tag_data = ?, "
        "revision = revision + 1 "
        "WHERE id = ? AND uid = ? AND (? = -1 OR revision = ?);");
    if (!update.IsValid()) {
      return LastStatus();
    }

    update.BindBlob(1, account.description);
    update.BindBlob(2, account.data);
    update.BindBlob(3, account.iv_description);
    update.BindBlob(4, account.iv_data);
    update.BindBlob(5, account.tag_description);
    update.BindBlob(6, account.tag_data);
    update.BindInt64(7, id);
    update.BindBlob(8, account.uid);
    update.BindInt64(9, expected_revision);
    update.BindInt64(10, expected_revision);

    if (update.Step() != SQLITE_DONE) {
      return LastStatus();
    }
  }

  if (sqlite3_changes(db_) == 0) {
    // Nothing matched: tell a stale revision apart from a missing account.
    Statement find(db_, "SELECT 1 FROM accounts WHERE id = ? AND uid = ?;");
    if (!find.IsValid()) {
      return LastStatus();
    }
    find.BindInt64(1, id);
    find.BindBlob(2, account.uid);
    const int step = find.Step();
    if (step == SQLITE_ROW) {
      return DBStatus::kConflict;
    }
    if (step != SQLITE_DONE) {
      return LastStatus();
    }
    return DBStatus::kNotFound;
  }

  return tx.Commit() ? DBStatus::kOk : LastStatus();
}

std::expected<std::int64_t, DBStatus> DB::PositionOf(std::int64_t id) {
  Statement find(db_, "SELECT position FROM accounts WHERE id = ?;");
  if (!find.IsValid()) {
    return std::unexpected(LastStatus());
  }
  find.BindInt64(1, id);
  const int step = find.Step();
  if (step == SQLITE_DONE) {
    return std::unexpected(DBStatus::kNotFound);
  }
  if (step != SQLITE_ROW) {
    return std::unexpected(LastStatus());
  }
  return find.ColumnInt64(0);
}

DBStatus DB::DeleteAccount(std::int64_t id) {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return DBStatusFromSqlite(tx.BeginResult());
  }

  const auto position = PositionOf(id);
  if (!position.has_value()) {
    return position.error();
  }

  {
    Statement del(db_, "DELETE FROM accounts WHERE id = ?;");
    if (!del.IsValid()) {
      return LastStatus();
    }
    del.BindInt64(1, id);
    if (del.Step() != SQLITE_DONE) {
      return LastStatus();
    }
  }

  Statement shift(
      db_, "UPDATE accounts SET position = position - 1 WHERE position > ?;");
  if (!shift.IsValid()) {
    return LastStatus();
  }
  shift.BindInt64(1, *position);
  if (shift.Step() != SQLITE_DONE) {
    return LastStatus();
  }

  return tx.Commit() ? DBStatus::kOk : LastStatus();
}

DBStatus DB::MoveAccount(std::int64_t id, std::int64_t to_position) {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }
  if (to_position < 0) {
    return DBStatus::kNotFound;
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return DBStatusFromSqlite(tx.BeginResult());
  }

  const auto from = PositionOf(id);
  if (!from.has_value()) {
    return from.error();
  }
  const std::int64_t from_position = *from;

  {
    Statement count(db_, "SELECT COUNT(*) FROM accounts;");
    if (!count.IsValid() || count.Step() != SQLITE_ROW) {
      return LastStatus();
    }
    if (to_position >= count.ColumnInt64(0)) {
      return DBStatus::kNotFound;
    }
  }

  if (from_position == to_position) {
    return tx.Commit() ? DBStatus::kOk : LastStatus();
  }

  const bool moving_down = to_position > from_position;
  const std::string_view shift_sql =
      moving_down ? "UPDATE accounts SET position = position - 1 "
                    "WHERE position > ? AND position <= ?;"
                  : "UPDATE accounts SET position = position + 1 "
                    "WHERE position >= ? AND position < ?;";

  Statement shift(db_, shift_sql);
  if (!shift.IsValid()) {
    return LastStatus();
  }
  if (moving_down) {
    shift.BindInt64(1, from_position);
    shift.BindInt64(2, to_position);
  } else {
    shift.BindInt64(1, to_position);
    shift.BindInt64(2, from_position);
  }
  if (shift.Step() != SQLITE_DONE) {
    return LastStatus();
  }

  Statement move(db_, "UPDATE accounts SET position = ? WHERE id = ?;");
  if (!move.IsValid()) {
    return LastStatus();
  }
  move.BindInt64(1, to_position);
  move.BindInt64(2, id);
  if (move.Step() != SQLITE_DONE) {
    return LastStatus();
  }

  return tx.Commit() ? DBStatus::kOk : LastStatus();
}

std::expected<std::vector<AccountIndex>, DBStatus> DB::GetAllIndex() {
  if (!IsOpen()) {
    return std::unexpected(DBStatus::kClosed);
  }

  Statement select(db_,
                   "SELECT id, position, revision, uid, description, "
                   "iv_description, tag_description FROM accounts "
                   "ORDER BY position;");
  if (!select.IsValid()) {
    return std::unexpected(LastStatus());
  }

  std::vector<AccountIndex> result;
  int step = SQLITE_ROW;
  while ((step = select.Step()) == SQLITE_ROW) {
    AccountIndex index;
    index.id = select.ColumnInt64(0);
    index.position = select.ColumnInt64(1);
    index.revision = select.ColumnInt64(2);
    const bool ok = select.ColumnBlob(3, index.uid) &&
                    select.ColumnBlob(4, index.description) &&
                    select.ColumnBlob(5, index.iv_description) &&
                    select.ColumnBlob(6, index.tag_description);
    if (!ok) {
      return std::unexpected(DBStatus::kError);
    }
    result.push_back(index);
  }
  if (step != SQLITE_DONE) {
    return std::unexpected(LastStatus());
  }

  return result;
}

std::expected<std::vector<std::pair<std::int64_t, AccountBinary>>, DBStatus>
DB::GetAllAccounts() {
  if (!IsOpen()) {
    return std::unexpected(DBStatus::kClosed);
  }

  Statement select(db_,
                   "SELECT id, uid, description, data, iv_description, "
                   "iv_data, tag_description, tag_data, revision "
                   "FROM accounts ORDER BY position;");
  if (!select.IsValid()) {
    return std::unexpected(LastStatus());
  }

  std::vector<std::pair<std::int64_t, AccountBinary>> result;
  int step = SQLITE_ROW;
  while ((step = select.Step()) == SQLITE_ROW) {
    AccountBinary account;
    const std::int64_t id = select.ColumnInt64(0);
    const bool ok = select.ColumnBlob(1, account.uid) &&
                    select.ColumnBlob(2, account.description) &&
                    select.ColumnBlob(3, account.data) &&
                    select.ColumnBlob(4, account.iv_description) &&
                    select.ColumnBlob(5, account.iv_data) &&
                    select.ColumnBlob(6, account.tag_description) &&
                    select.ColumnBlob(7, account.tag_data);
    if (!ok) {
      return std::unexpected(DBStatus::kError);
    }
    account.revision = select.ColumnInt64(8);
    result.emplace_back(id, account);
  }
  if (step != SQLITE_DONE) {
    return std::unexpected(LastStatus());
  }

  return result;
}

DBStatus DB::RunInTransaction(const std::function<DBStatus()>& body) {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return DBStatusFromSqlite(tx.BeginResult());
  }

  const DBStatus status = body();
  if (status != DBStatus::kOk) {
    return status;
  }
  return tx.Commit() ? DBStatus::kOk : LastStatus();
}

std::expected<std::size_t, DBStatus> DB::Count() {
  if (!IsOpen()) {
    return std::unexpected(DBStatus::kClosed);
  }

  Statement count(db_, "SELECT COUNT(*) FROM accounts;");
  if (!count.IsValid() || count.Step() != SQLITE_ROW) {
    return std::unexpected(LastStatus());
  }

  return static_cast<std::size_t>(count.ColumnInt64(0));
}

DBStatus DB::Compact() {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }
  return Execute("VACUUM;") ? DBStatus::kOk : LastStatus();
}

std::expected<VaultMeta, DBStatus> DB::GetMeta() {
  if (!IsOpen()) {
    return std::unexpected(DBStatus::kClosed);
  }

  Statement select(db_,
                   "SELECT version, kdf_salt, kdf_memory_kib, kdf_iterations, "
                   "kdf_parallelism, verifier, verifier_iv, verifier_tag, "
                   "key_epoch FROM vault_meta WHERE singleton = 0;");
  if (!select.IsValid()) {
    return std::unexpected(LastStatus());
  }

  const int step = select.Step();
  if (step == SQLITE_DONE) {
    return std::unexpected(DBStatus::kNotFound);
  }
  if (step != SQLITE_ROW) {
    return std::unexpected(LastStatus());
  }

  const auto to_u32 = [](std::int64_t value, std::uint32_t& out) {
    if (!std::in_range<std::uint32_t>(value)) {
      return false;
    }
    out = static_cast<std::uint32_t>(value);
    return true;
  };

  VaultMeta meta;
  meta.version = select.ColumnInt64(0);
  meta.key_epoch = select.ColumnInt64(8);
  const bool ok = select.ColumnBlob(1, meta.kdf_salt) &&
                  to_u32(select.ColumnInt64(2), meta.kdf_params.memory_kib) &&
                  to_u32(select.ColumnInt64(3), meta.kdf_params.iterations) &&
                  to_u32(select.ColumnInt64(4), meta.kdf_params.parallelism) &&
                  select.ColumnBlob(5, meta.verifier) &&
                  select.ColumnBlob(6, meta.verifier_iv) &&
                  select.ColumnBlob(7, meta.verifier_tag);
  if (!ok) {
    return std::unexpected(DBStatus::kError);
  }
  return meta;
}

DBStatus DB::CreateMeta(const VaultMeta& meta) {
  return WriteMeta(
      "INSERT INTO vault_meta (version, kdf_salt, kdf_memory_kib, "
      "kdf_iterations, kdf_parallelism, verifier, verifier_iv, verifier_tag, "
      "key_epoch, singleton) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 0);",
      meta);
}

DBStatus DB::UpdateMeta(const VaultMeta& meta) {
  const DBStatus status = WriteMeta(
      "UPDATE vault_meta SET version = ?, kdf_salt = ?, kdf_memory_kib = ?, "
      "kdf_iterations = ?, kdf_parallelism = ?, verifier = ?, "
      "verifier_iv = ?, verifier_tag = ?, key_epoch = ? "
      "WHERE singleton = 0;",
      meta);
  if (status != DBStatus::kOk) {
    return status;
  }
  return sqlite3_changes(db_) > 0 ? DBStatus::kOk : DBStatus::kNotFound;
}

DBStatus DB::WriteMeta(std::string_view sql, const VaultMeta& meta) {
  if (!IsOpen()) {
    return DBStatus::kClosed;
  }

  Statement write(db_, sql);
  if (!write.IsValid()) {
    return LastStatus();
  }

  write.BindInt64(1, meta.version);
  write.BindBlob(2, meta.kdf_salt);
  write.BindInt64(3, meta.kdf_params.memory_kib);
  write.BindInt64(4, meta.kdf_params.iterations);
  write.BindInt64(5, meta.kdf_params.parallelism);
  write.BindBlob(6, meta.verifier);
  write.BindBlob(7, meta.verifier_iv);
  write.BindBlob(8, meta.verifier_tag);
  write.BindInt64(9, meta.key_epoch);

  if (write.Step() != SQLITE_DONE) {
    return LastStatus();
  }
  return DBStatus::kOk;
}
