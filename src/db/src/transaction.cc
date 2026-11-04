#include "db/transaction.h"

#include <sqlite3.h>

namespace {

int Begin(sqlite3* db, bool nested) noexcept {
  if (db == nullptr) {
    return SQLITE_MISUSE;
  }
  if (nested) {
    return SQLITE_OK;
  }
  return sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr);
}

}  // namespace

Transaction::Transaction(sqlite3* db) noexcept
    : db_(db),
      nested_(db != nullptr && sqlite3_get_autocommit(db) == 0),
      begin_result_(Begin(db, nested_)) {
  active_ = begin_result_ == SQLITE_OK;
}

Transaction::~Transaction() {
  if (active_ && !nested_) {
    sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
  }
}

bool Transaction::IsActive() const noexcept {
  return active_;
}

int Transaction::BeginResult() const noexcept {
  return begin_result_;
}

bool Transaction::IsNested() const noexcept {
  return nested_;
}

bool Transaction::Commit() noexcept {
  if (!active_) {
    return false;
  }
  if (nested_) {
    active_ = false;
    return true;
  }
  if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK) {
    active_ = false;
    return true;
  }
  // A failed COMMIT (e.g. SQLITE_BUSY) leaves the transaction open; roll it
  // back so the connection is usable again and nothing is half-applied.
  sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
  active_ = false;
  return false;
}
