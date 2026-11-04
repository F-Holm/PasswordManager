#pragma once

struct sqlite3;

// RAII guard around a SQLite write transaction.
//
// The constructor issues `BEGIN IMMEDIATE`, which takes the database's write
// lock up front (waiting up to the connection's busy timeout). If it cannot
// be taken, IsActive() is false and the caller must abort without touching
// the database: running the statements anyway would apply them one by one in
// autocommit mode, so a later failure could no longer be rolled back.
//
// Unless Commit() succeeds, the destructor rolls the transaction back.
//
// If the connection is already inside a transaction, the guard joins it
// instead: it is active, Commit() succeeds without committing and nothing is
// rolled back on destruction, leaving both decisions to the outermost guard.
class Transaction {
 public:
  explicit Transaction(sqlite3* db) noexcept;
  ~Transaction();

  Transaction(const Transaction&) = delete;
  Transaction& operator=(const Transaction&) = delete;
  Transaction(Transaction&&) = delete;
  Transaction& operator=(Transaction&&) = delete;

  // True while the transaction is open (began and not yet committed or
  // rolled back).
  [[nodiscard]] bool IsActive() const noexcept;
  // True if this guard joined an enclosing transaction.
  [[nodiscard]] bool IsNested() const noexcept;
  // SQLite result code of the `BEGIN IMMEDIATE` statement.
  [[nodiscard]] int BeginResult() const noexcept;
  // Commits the transaction. Returns false (and rolls back) if the
  // transaction is not active or the commit fails.
  bool Commit() noexcept;

 private:
  sqlite3* db_;
  bool nested_;
  int begin_result_;
  bool active_ = false;
};
