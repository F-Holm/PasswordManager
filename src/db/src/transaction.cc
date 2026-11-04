#include "db/transaction.h"

#include <sqlite3.h>

Transaction::Transaction(sqlite3* db) noexcept
    : db_(db),
      begin_result_(db == nullptr ? SQLITE_MISUSE
                                  : sqlite3_exec(db, "BEGIN IMMEDIATE;",
                                                 nullptr, nullptr, nullptr)) {
  active_ = begin_result_ == SQLITE_OK;
}

Transaction::~Transaction() {
  if (active_) {
    sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
  }
}

bool Transaction::IsActive() const noexcept {
  return active_;
}

int Transaction::BeginResult() const noexcept {
  return begin_result_;
}

bool Transaction::Commit() noexcept {
  if (!active_) {
    return false;
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
