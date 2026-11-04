#include "db/db.h"

#include <sqlite3.h>

#include <string>

#include "db/statement.h"
#include "db/transaction.h"

DB::DB() noexcept = default;

DB::~DB() {
  Close();
}

bool DB::Open(std::string_view path) {
  Close();

  const std::string path_str(path);
  if (sqlite3_open(path_str.c_str(), &db_) != SQLITE_OK) {
    Close();
    return false;
  }

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

bool DB::Execute(std::string_view sql) {
  const std::string sql_str(sql);
  return sqlite3_exec(db_, sql_str.c_str(), nullptr, nullptr, nullptr) ==
         SQLITE_OK;
}

bool DB::CreateSchema() {
  return Execute(
      "CREATE TABLE IF NOT EXISTS accounts ("
      "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  position INTEGER NOT NULL,"
      "  description BLOB NOT NULL,"
      "  data BLOB NOT NULL,"
      "  iv_description BLOB NOT NULL,"
      "  iv_data BLOB NOT NULL,"
      "  tag_description BLOB NOT NULL,"
      "  tag_data BLOB NOT NULL"
      ");"
      "CREATE INDEX IF NOT EXISTS idx_accounts_position "
      "  ON accounts(position);");
}

bool DB::AddAccount(const AccountBinary& account) {
  if (!IsOpen()) {
    return false;
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return false;
  }

  Statement insert(db_,
                   "INSERT INTO accounts (position, description, data, "
                   "iv_description, iv_data, tag_description, tag_data) "
                   "VALUES ("
                   "  (SELECT COALESCE(MAX(position), -1) + 1 "
                   "     FROM accounts),"
                   "  ?, ?, ?, ?, ?, ?);");
  if (!insert.IsValid()) {
    return false;
  }

  insert.BindBlob(1, account.description);
  insert.BindBlob(2, account.data);
  insert.BindBlob(3, account.iv_description);
  insert.BindBlob(4, account.iv_data);
  insert.BindBlob(5, account.tag_description);
  insert.BindBlob(6, account.tag_data);

  if (insert.Step() != SQLITE_DONE) {
    return false;
  }

  return tx.Commit();
}

std::optional<AccountBinary> DB::GetAccount(std::int64_t position) {
  if (!IsOpen()) {
    return std::nullopt;
  }

  Statement select(db_,
                   "SELECT description, data, iv_description, iv_data, "
                   "tag_description, tag_data FROM accounts "
                   "WHERE position = ?;");
  if (!select.IsValid()) {
    return std::nullopt;
  }

  select.BindInt64(1, position);

  if (select.Step() != SQLITE_ROW) {
    return std::nullopt;
  }

  AccountBinary account;
  const bool ok = select.ColumnBlob(0, account.description) &&
                  select.ColumnBlob(1, account.data) &&
                  select.ColumnBlob(2, account.iv_description) &&
                  select.ColumnBlob(3, account.iv_data) &&
                  select.ColumnBlob(4, account.tag_description) &&
                  select.ColumnBlob(5, account.tag_data);
  if (!ok) {
    return std::nullopt;
  }

  return account;
}

bool DB::UpdateAccount(std::int64_t position, const AccountBinary& account) {
  if (!IsOpen()) {
    return false;
  }

  Statement update(db_,
                   "UPDATE accounts SET description = ?, data = ?, "
                   "iv_description = ?, iv_data = ?, tag_description = ?, "
                   "tag_data = ? WHERE position = ?;");
  if (!update.IsValid()) {
    return false;
  }

  update.BindBlob(1, account.description);
  update.BindBlob(2, account.data);
  update.BindBlob(3, account.iv_description);
  update.BindBlob(4, account.iv_data);
  update.BindBlob(5, account.tag_description);
  update.BindBlob(6, account.tag_data);
  update.BindInt64(7, position);

  if (update.Step() != SQLITE_DONE) {
    return false;
  }

  return sqlite3_changes(db_) > 0;
}

bool DB::DeleteAccount(std::int64_t position) {
  if (!IsOpen()) {
    return false;
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return false;
  }

  bool deleted = false;
  {
    Statement del(db_, "DELETE FROM accounts WHERE position = ?;");
    if (!del.IsValid()) {
      return false;
    }
    del.BindInt64(1, position);
    if (del.Step() != SQLITE_DONE) {
      return false;
    }
    deleted = sqlite3_changes(db_) > 0;
  }

  if (!deleted) {
    return false;
  }

  Statement shift(
      db_, "UPDATE accounts SET position = position - 1 WHERE position > ?;");
  if (!shift.IsValid()) {
    return false;
  }
  shift.BindInt64(1, position);
  if (shift.Step() != SQLITE_DONE) {
    return false;
  }

  return tx.Commit();
}

bool DB::MoveAccount(std::int64_t from_position, std::int64_t to_position) {
  if (!IsOpen() || from_position < 0 || to_position < 0) {
    return false;
  }

  if (from_position == to_position) {
    return GetAccount(from_position).has_value();
  }

  Transaction tx(db_);
  if (!tx.IsActive()) {
    return false;
  }

  std::int64_t moving_id = -1;
  {
    Statement find(db_, "SELECT id FROM accounts WHERE position = ?;");
    if (!find.IsValid()) {
      return false;
    }
    find.BindInt64(1, from_position);
    if (find.Step() != SQLITE_ROW) {
      return false;
    }
    moving_id = find.ColumnInt64(0);
  }

  {
    Statement count(db_, "SELECT COUNT(*) FROM accounts;");
    if (!count.IsValid() || count.Step() != SQLITE_ROW) {
      return false;
    }
    if (to_position >= count.ColumnInt64(0)) {
      return false;
    }
  }

  const bool moving_down = to_position > from_position;
  const std::string_view shift_sql =
      moving_down ? "UPDATE accounts SET position = position - 1 "
                    "WHERE position > ? AND position <= ?;"
                  : "UPDATE accounts SET position = position + 1 "
                    "WHERE position >= ? AND position < ?;";

  Statement shift(db_, shift_sql);
  if (!shift.IsValid()) {
    return false;
  }
  if (moving_down) {
    shift.BindInt64(1, from_position);
    shift.BindInt64(2, to_position);
  } else {
    shift.BindInt64(1, to_position);
    shift.BindInt64(2, from_position);
  }
  if (shift.Step() != SQLITE_DONE) {
    return false;
  }

  Statement move(db_, "UPDATE accounts SET position = ? WHERE id = ?;");
  if (!move.IsValid()) {
    return false;
  }
  move.BindInt64(1, to_position);
  move.BindInt64(2, moving_id);
  if (move.Step() != SQLITE_DONE) {
    return false;
  }

  return tx.Commit();
}

std::vector<AccountIndex> DB::GetAllIndex() {
  std::vector<AccountIndex> result;
  if (!IsOpen()) {
    return result;
  }

  Statement select(db_,
                   "SELECT position, description, iv_description, "
                   "tag_description FROM accounts ORDER BY position;");
  if (!select.IsValid()) {
    return result;
  }

  while (select.Step() == SQLITE_ROW) {
    AccountIndex index;
    index.position = select.ColumnInt64(0);
    select.ColumnBlob(1, index.description);
    select.ColumnBlob(2, index.iv_description);
    select.ColumnBlob(3, index.tag_description);
    result.push_back(index);
  }

  return result;
}

std::size_t DB::Count() {
  if (!IsOpen()) {
    return 0;
  }

  Statement count(db_, "SELECT COUNT(*) FROM accounts;");
  if (!count.IsValid() || count.Step() != SQLITE_ROW) {
    return 0;
  }

  return static_cast<std::size_t>(count.ColumnInt64(0));
}

bool DB::Compact() {
  if (!IsOpen()) {
    return false;
  }
  return Execute("VACUUM;");
}
