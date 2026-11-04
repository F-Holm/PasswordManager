#include "db/db_status.h"

#include <sqlite3.h>

DBStatus DBStatusFromSqlite(int result_code) noexcept {
  switch (result_code & 0xFF) {  // Strip extended result codes.
    case SQLITE_OK:
    case SQLITE_DONE:
    case SQLITE_ROW:
      return DBStatus::kOk;
    case SQLITE_BUSY:
    case SQLITE_LOCKED:
      return DBStatus::kBusy;
    case SQLITE_CONSTRAINT:
      return DBStatus::kAlreadyExists;
    case SQLITE_NOTFOUND:
      return DBStatus::kNotFound;
    default:
      return DBStatus::kError;
  }
}
