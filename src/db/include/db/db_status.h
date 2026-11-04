#pragma once

#include <cstdint>

// Outcome of a DB operation.
enum class DBStatus : std::uint8_t {
  kOk,
  // The requested account (or position) does not exist.
  kNotFound,
  // Another connection held the database lock for longer than the busy
  // timeout. Nothing was changed; the operation can be retried.
  kBusy,
  // The database is not open.
  kClosed,
  // Any other SQLite failure (corrupted file, I/O error, bad schema, ...).
  kError,
};

// Maps a SQLite result code to the DBStatus a caller should act on.
DBStatus DBStatusFromSqlite(int result_code) noexcept;
