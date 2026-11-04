#include "db/db.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>

#include "types/account_binary.h"
#include "types/vault_meta.h"

namespace {

// `uid_marker` defaults to `marker`, so accounts made with different
// markers have distinct uids (the column is UNIQUE).
AccountBinary MakeAccount(std::byte marker, std::byte uid_marker) {
  AccountBinary account;
  account.uid.fill(uid_marker);
  account.description.fill(marker);
  account.data.fill(marker);
  account.iv_description.fill(marker);
  account.iv_data.fill(marker);
  account.tag_description.fill(marker);
  account.tag_data.fill(marker);
  return account;
}

AccountBinary MakeAccount(std::byte marker) {
  return MakeAccount(marker, marker);
}

// Id of the account at `position`, or -1 if there is none. Lets the tests
// below speak in positions, which are easier to follow.
std::int64_t IdAt(DB& db, std::int64_t position) {
  const auto index = db.GetAllIndex();
  if (index.has_value()) {
    for (const AccountIndex& entry : *index) {
      if (entry.position == position) {
        return entry.id;
      }
    }
  }
  return -1;
}

VaultMeta MakeMeta(std::byte marker) {
  VaultMeta meta;
  meta.kdf_salt.fill(marker);
  meta.kdf_params = {.memory_kib = 1024, .iterations = 2, .parallelism = 3};
  meta.verifier.fill(marker);
  meta.verifier_iv.fill(marker);
  meta.verifier_tag.fill(marker);
  meta.key_epoch = 7;
  return meta;
}

}  // namespace

class DBTest : public ::testing::Test {
 protected:
  // Each ctest case runs as a separate process of the same binary sharing
  // the working directory, so the file name must be unique per test to stay
  // parallel-safe (see `just test`).
  const std::string test_file =
      std::string("test_database_") +
      ::testing::UnitTest::GetInstance()->current_test_info()->name() +
      ".sqlite3";

  void SetUp() override {
    std::remove(test_file.c_str());
  }

  void TearDown() override {
    std::remove(test_file.c_str());
  }
};

TEST_F(DBTest, OpenCreatesEmptyDatabase) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_TRUE(db.IsOpen());
  EXPECT_EQ(db.Count().value(), 0u);
}

TEST_F(DBTest, OpenFailsForUnreachablePath) {
  DB db;
  EXPECT_FALSE(db.Open("/nonexistent_dir_xyz/unreachable.sqlite3"));
  EXPECT_FALSE(db.IsOpen());
}

TEST_F(DBTest, OpenFailsWhenSchemaCannotBeWritten) {
  {
    std::ofstream ofs(test_file, std::ios::binary | std::ios::trunc);
  }
  std::filesystem::permissions(test_file, std::filesystem::perms::owner_read);

  DB db;
  EXPECT_FALSE(db.Open(test_file));
  EXPECT_FALSE(db.IsOpen());

  std::filesystem::permissions(test_file, std::filesystem::perms::owner_all);
}

TEST_F(DBTest, OperationsFailWhenDatabaseNotOpen) {
  DB db;
  ASSERT_FALSE(db.IsOpen());

  EXPECT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kClosed);
  EXPECT_EQ(db.GetAccount(IdAt(db, 0)).error(), DBStatus::kClosed);
  EXPECT_EQ(db.UpdateAccount(IdAt(db, 0), MakeAccount(std::byte{0x01})),
            DBStatus::kClosed);
  EXPECT_EQ(db.DeleteAccount(IdAt(db, 0)), DBStatus::kClosed);
  EXPECT_EQ(db.MoveAccount(IdAt(db, 0), 1), DBStatus::kClosed);
  EXPECT_EQ(db.GetAllIndex().error(), DBStatus::kClosed);
  EXPECT_EQ(db.Count().error(), DBStatus::kClosed);
  EXPECT_EQ(db.Compact(), DBStatus::kClosed);
}

TEST_F(DBTest, AddAndGetAccount) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  AccountBinary account = MakeAccount(std::byte{0xAB});
  ASSERT_EQ(db.AddAccount(account), DBStatus::kOk);
  EXPECT_EQ(db.Count().value(), 1u);

  auto fetched = db.GetAccount(IdAt(db, 0));
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(std::memcmp(fetched->description.data(), account.description.data(),
                        account.description.size()),
            0);
  EXPECT_EQ(std::memcmp(fetched->data.data(), account.data.data(),
                        account.data.size()),
            0);
}

TEST_F(DBTest, GetMissingAccountReturnsNotFound) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.GetAccount(IdAt(db, 0)).error(), DBStatus::kNotFound);
}

TEST_F(DBTest, GetAccountReturnsNulloptWhenBlobIsCorrupted) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);

  // Simulate on-disk corruption (or tampering) by rewriting a blob column to
  // the wrong length through a second raw connection to the same file.
  sqlite3* raw = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &raw), SQLITE_OK);
  ASSERT_EQ(sqlite3_exec(raw,
                         "UPDATE accounts SET description = X'00' "
                         "WHERE position = 0;",
                         nullptr, nullptr, nullptr),
            SQLITE_OK);
  sqlite3_close(raw);

  EXPECT_FALSE(db.GetAccount(IdAt(db, 0)).has_value());
}

TEST_F(DBTest, AddAppendsAtIncrementingPositions) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x03})), DBStatus::kOk);

  ASSERT_TRUE(db.GetAccount(IdAt(db, 0)).has_value());
  ASSERT_TRUE(db.GetAccount(IdAt(db, 1)).has_value());
  ASSERT_TRUE(db.GetAccount(IdAt(db, 2)).has_value());
  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(IdAt(db, 1))->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(IdAt(db, 2))->description[0], std::byte{0x03});
}

TEST_F(DBTest, UpdateAccountOverwritesExistingRecord) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.UpdateAccount(IdAt(db, 0),
                             MakeAccount(std::byte{0x02}, std::byte{0x01})),
            DBStatus::kOk);

  auto fetched = db.GetAccount(IdAt(db, 0));
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(fetched->description[0], std::byte{0x02});
  EXPECT_EQ(fetched->uid[0], std::byte{0x01});
}

TEST_F(DBTest, UpdateAccountRejectsADifferentUid) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  EXPECT_EQ(db.UpdateAccount(IdAt(db, 0), MakeAccount(std::byte{0x02})),
            DBStatus::kNotFound);
  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x01});
}

TEST_F(DBTest, AddAccountRejectsADuplicateUid) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  EXPECT_EQ(db.AddAccount(MakeAccount(std::byte{0x02}, std::byte{0x01})),
            DBStatus::kAlreadyExists);
  EXPECT_EQ(db.Count().value(), 1u);
}

TEST_F(DBTest, UidRoundTripsThroughGetAccountAndIndex) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01}, std::byte{0x7E})),
            DBStatus::kOk);

  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->uid[0], std::byte{0x7E});
  EXPECT_EQ(db.GetAllIndex().value()[0].uid[0], std::byte{0x7E});
}

TEST_F(DBTest, UpdateMissingAccountFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.UpdateAccount(IdAt(db, 0), MakeAccount(std::byte{0x02})),
            DBStatus::kNotFound);
}

TEST_F(DBTest, DeleteAccountShiftsFollowingPositions) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x03})), DBStatus::kOk);

  ASSERT_EQ(db.DeleteAccount(IdAt(db, 0)), DBStatus::kOk);

  EXPECT_EQ(db.Count().value(), 2u);
  ASSERT_TRUE(db.GetAccount(IdAt(db, 0)).has_value());
  ASSERT_TRUE(db.GetAccount(IdAt(db, 1)).has_value());
  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(IdAt(db, 1))->description[0], std::byte{0x03});
  EXPECT_FALSE(db.GetAccount(IdAt(db, 2)).has_value());
}

TEST_F(DBTest, DeleteMissingAccountFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.DeleteAccount(IdAt(db, 0)), DBStatus::kNotFound);
}

TEST_F(DBTest, MoveAccountForward) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x03})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x04})), DBStatus::kOk);

  // Move item at 0 to 2: expected order becomes 2, 3, 1, 4
  ASSERT_EQ(db.MoveAccount(IdAt(db, 0), 2), DBStatus::kOk);

  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(IdAt(db, 1))->description[0], std::byte{0x03});
  EXPECT_EQ(db.GetAccount(IdAt(db, 2))->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(IdAt(db, 3))->description[0], std::byte{0x04});
}

TEST_F(DBTest, MoveAccountBackward) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x03})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x04})), DBStatus::kOk);

  // Move item at 3 to 1: expected order becomes 1, 4, 2, 3
  ASSERT_EQ(db.MoveAccount(IdAt(db, 3), 1), DBStatus::kOk);

  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(IdAt(db, 1))->description[0], std::byte{0x04});
  EXPECT_EQ(db.GetAccount(IdAt(db, 2))->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(IdAt(db, 3))->description[0], std::byte{0x03});
}

TEST_F(DBTest, MoveAccountToSamePositionIsNoop) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);

  EXPECT_EQ(db.MoveAccount(IdAt(db, 1), 1), DBStatus::kOk);

  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(IdAt(db, 1))->description[0], std::byte{0x02});
}

TEST_F(DBTest, MoveAccountOutOfRangeFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);

  EXPECT_EQ(db.MoveAccount(IdAt(db, 0), 5), DBStatus::kNotFound);
  EXPECT_EQ(db.MoveAccount(IdAt(db, -1), 0), DBStatus::kNotFound);
}

TEST_F(DBTest, MoveAccountFromMissingPositionFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);

  EXPECT_EQ(db.MoveAccount(IdAt(db, 50), 0), DBStatus::kNotFound);
}

TEST_F(DBTest, GetAllIndexReturnsOrderedEntries) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x03})), DBStatus::kOk);

  auto index = db.GetAllIndex().value();
  ASSERT_EQ(index.size(), 3u);
  EXPECT_EQ(index[0].position, 0);
  EXPECT_EQ(index[1].position, 1);
  EXPECT_EQ(index[2].position, 2);
  EXPECT_EQ(index[0].description[0], std::byte{0x01});
  EXPECT_EQ(index[0].iv_description[0], std::byte{0x01});
  EXPECT_EQ(index[0].tag_description[0], std::byte{0x01});
}

TEST_F(DBTest, CompactKeepsDataIntact) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.DeleteAccount(IdAt(db, 0)), DBStatus::kOk);

  EXPECT_EQ(db.Compact(), DBStatus::kOk);
  EXPECT_EQ(db.Count().value(), 1u);
  ASSERT_TRUE(db.GetAccount(IdAt(db, 0)).has_value());
  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x02});
}

TEST_F(DBTest, OperationsFailGracefullyIfSchemaIsCorrupted) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);

  // Simulate the vault file being tampered with (or corrupted) externally:
  // drop the table out from under a live DB instance via a second raw
  // connection, then verify every operation fails cleanly instead of
  // crashing on an unpreparable statement.
  sqlite3* raw = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &raw), SQLITE_OK);
  ASSERT_EQ(
      sqlite3_exec(raw, "DROP TABLE accounts;", nullptr, nullptr, nullptr),
      SQLITE_OK);
  sqlite3_close(raw);

  EXPECT_NE(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  EXPECT_FALSE(db.GetAccount(IdAt(db, 0)).has_value());
  EXPECT_NE(db.UpdateAccount(IdAt(db, 0), MakeAccount(std::byte{0x02})),
            DBStatus::kOk);
  EXPECT_NE(db.DeleteAccount(IdAt(db, 0)), DBStatus::kOk);
  EXPECT_NE(db.MoveAccount(IdAt(db, 0), 1), DBStatus::kOk);
  EXPECT_FALSE(db.GetAllIndex().has_value());
  EXPECT_FALSE(db.Count().has_value());
}

TEST_F(DBTest, PersistenceBetweenSessions) {
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x2A})), DBStatus::kOk);
  }

  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    auto fetched = db.GetAccount(IdAt(db, 0));
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched->description[0], std::byte{0x2A});
  }
}

TEST_F(DBTest, WritesFailCleanlyWhileAnotherConnectionHoldsTheWriteLock) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);

  // Do not wait for the lock: this test is about failing cleanly.
  ASSERT_TRUE(db.SetBusyTimeout(0));

  sqlite3* other = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &other), SQLITE_OK);
  ASSERT_EQ(sqlite3_exec(other, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr),
            SQLITE_OK);

  EXPECT_EQ(db.AddAccount(MakeAccount(std::byte{0x03})), DBStatus::kBusy);
  EXPECT_EQ(db.DeleteAccount(IdAt(db, 0)), DBStatus::kBusy);
  EXPECT_EQ(db.MoveAccount(IdAt(db, 0), 1), DBStatus::kBusy);
  EXPECT_EQ(db.UpdateAccount(IdAt(db, 0), MakeAccount(std::byte{0x03})),
            DBStatus::kBusy);

  sqlite3_exec(other, "ROLLBACK;", nullptr, nullptr, nullptr);
  sqlite3_close(other);

  // Nothing was half-applied: both accounts are still at their positions.
  ASSERT_EQ(db.Count().value(), 2u);
  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(IdAt(db, 1))->description[0], std::byte{0x02});
}

TEST_F(DBTest, WritesWaitForAnotherConnectionToReleaseTheLock) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  sqlite3* other = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &other), SQLITE_OK);
  ASSERT_EQ(sqlite3_exec(other, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr),
            SQLITE_OK);

  // Release the lock well within DB::kDefaultBusyTimeoutMs.
  std::thread releaser([other] {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sqlite3_exec(other, "COMMIT;", nullptr, nullptr, nullptr);
  });

  EXPECT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);

  releaser.join();
  sqlite3_close(other);
  EXPECT_EQ(db.Count().value(), 1u);
}

TEST_F(DBTest, SetBusyTimeoutFailsWhenClosedOrNegative) {
  DB db;
  EXPECT_FALSE(db.SetBusyTimeout(100));
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_FALSE(db.SetBusyTimeout(-1));
  EXPECT_TRUE(db.SetBusyTimeout(100));
}

TEST(DBStatusTest, MapsSqliteResultCodes) {
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_OK), DBStatus::kOk);
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_DONE), DBStatus::kOk);
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_BUSY), DBStatus::kBusy);
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_BUSY_SNAPSHOT), DBStatus::kBusy);
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_LOCKED), DBStatus::kBusy);
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_CORRUPT), DBStatus::kError);
  EXPECT_EQ(DBStatusFromSqlite(SQLITE_IOERR_READ), DBStatus::kError);
}

TEST_F(DBTest, NewVaultHasNoMeta) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.GetMeta().error(), DBStatus::kNotFound);
}

TEST_F(DBTest, CreateAndGetMetaRoundTrip) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  const VaultMeta meta = MakeMeta(std::byte{0x11});
  ASSERT_EQ(db.CreateMeta(meta), DBStatus::kOk);

  const auto fetched = db.GetMeta();
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(*fetched, meta);
}

TEST_F(DBTest, CreateMetaTwiceFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.CreateMeta(MakeMeta(std::byte{0x11})), DBStatus::kOk);
  EXPECT_EQ(db.CreateMeta(MakeMeta(std::byte{0x22})), DBStatus::kAlreadyExists);
  EXPECT_EQ(db.GetMeta()->kdf_salt[0], std::byte{0x11});
}

TEST_F(DBTest, UpdateMetaOverwritesIt) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.CreateMeta(MakeMeta(std::byte{0x11})), DBStatus::kOk);

  VaultMeta updated = MakeMeta(std::byte{0x22});
  updated.key_epoch = 8;
  ASSERT_EQ(db.UpdateMeta(updated), DBStatus::kOk);
  EXPECT_EQ(*db.GetMeta(), updated);
}

TEST_F(DBTest, UpdateMetaWithoutMetaFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.UpdateMeta(MakeMeta(std::byte{0x22})), DBStatus::kNotFound);
}

TEST_F(DBTest, MetaPersistsBetweenSessions) {
  const VaultMeta meta = MakeMeta(std::byte{0x33});
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    ASSERT_EQ(db.CreateMeta(meta), DBStatus::kOk);
  }
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(*db.GetMeta(), meta);
}

TEST_F(DBTest, MetaOperationsFailWhenClosed) {
  DB db;
  EXPECT_EQ(db.GetMeta().error(), DBStatus::kClosed);
  EXPECT_EQ(db.CreateMeta(MakeMeta(std::byte{0x01})), DBStatus::kClosed);
  EXPECT_EQ(db.UpdateMeta(MakeMeta(std::byte{0x01})), DBStatus::kClosed);
}

TEST_F(DBTest, GetMetaRejectsOutOfRangeKdfParams) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.CreateMeta(MakeMeta(std::byte{0x11})), DBStatus::kOk);

  sqlite3* raw = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &raw), SQLITE_OK);
  ASSERT_EQ(sqlite3_exec(raw, "UPDATE vault_meta SET kdf_iterations = -1;",
                         nullptr, nullptr, nullptr),
            SQLITE_OK);
  sqlite3_close(raw);

  EXPECT_EQ(db.GetMeta().error(), DBStatus::kError);
}

TEST_F(DBTest, OpenExistingOnlyFailsForMissingFile) {
  DB db;
  EXPECT_FALSE(db.Open(test_file, DB::OpenMode::kExistingOnly));
  EXPECT_FALSE(db.IsOpen());
  EXPECT_FALSE(std::filesystem::exists(test_file));
}

TEST_F(DBTest, OpenExistingOnlyOpensExistingFile) {
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
  }
  DB db;
  EXPECT_TRUE(db.Open(test_file, DB::OpenMode::kExistingOnly));
}

TEST_F(DBTest, GetAllAccountsReturnsEveryRowInOrder) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
  ASSERT_EQ(db.MoveAccount(IdAt(db, 1), 0), DBStatus::kOk);

  const auto accounts = db.GetAllAccounts();
  ASSERT_TRUE(accounts.has_value());
  ASSERT_EQ(accounts->size(), 2u);
  EXPECT_EQ((*accounts)[0].first, 2);  // The id of the second account added.
  EXPECT_EQ((*accounts)[0].second.data[0], std::byte{0x02});
  EXPECT_EQ((*accounts)[0].second.uid[0], std::byte{0x02});
  EXPECT_EQ((*accounts)[1].first, 1);
  EXPECT_EQ((*accounts)[1].second.data[0], std::byte{0x01});
}

TEST_F(DBTest, RunInTransactionCommitsOnSuccess) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.RunInTransaction([&] {
    EXPECT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);
    return db.AddAccount(MakeAccount(std::byte{0x02}));
  }),
            DBStatus::kOk);
  EXPECT_EQ(db.Count().value(), 2u);
}

TEST_F(DBTest, RunInTransactionRollsBackEverythingOnFailure) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01})), DBStatus::kOk);

  EXPECT_EQ(db.RunInTransaction([&] {
    EXPECT_EQ(db.AddAccount(MakeAccount(std::byte{0x02})), DBStatus::kOk);
    EXPECT_EQ(db.DeleteAccount(IdAt(db, 0)), DBStatus::kOk);
    return DBStatus::kError;
  }),
            DBStatus::kError);

  EXPECT_EQ(db.Count().value(), 1u);
  EXPECT_EQ(db.GetAccount(IdAt(db, 0))->data[0], std::byte{0x01});
}

TEST_F(DBTest, RunInTransactionFailsWhenClosed) {
  DB db;
  bool ran = false;
  EXPECT_EQ(db.RunInTransaction([&] {
    ran = true;
    return DBStatus::kOk;
  }),
            DBStatus::kClosed);
  EXPECT_FALSE(ran);
}

TEST_F(DBTest, AddAccountReportsItsId) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  std::int64_t first = -1;
  std::int64_t second = -1;
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01}), &first), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02}), &second),
            DBStatus::kOk);
  EXPECT_NE(first, second);
  EXPECT_EQ(db.GetAccount(first)->data[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(second)->data[0], std::byte{0x02});
}

TEST_F(DBTest, IdsSurviveDeletesAndMoves) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  std::int64_t a = -1;
  std::int64_t b = -1;
  std::int64_t c = -1;
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x0A}), &a), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x0B}), &b), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x0C}), &c), DBStatus::kOk);

  ASSERT_EQ(db.DeleteAccount(a), DBStatus::kOk);
  ASSERT_EQ(db.MoveAccount(c, 0), DBStatus::kOk);

  // Each id still names the same account, wherever it ended up.
  EXPECT_EQ(db.GetAccount(a).error(), DBStatus::kNotFound);
  EXPECT_EQ(db.GetAccount(b)->data[0], std::byte{0x0B});
  EXPECT_EQ(db.GetAccount(c)->data[0], std::byte{0x0C});

  const auto index = db.GetAllIndex().value();
  ASSERT_EQ(index.size(), 2u);
  EXPECT_EQ(index[0].id, c);
  EXPECT_EQ(index[0].position, 0);
  EXPECT_EQ(index[1].id, b);
  EXPECT_EQ(index[1].position, 1);
}

TEST_F(DBTest, IdsAreNotReused) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  std::int64_t first = -1;
  std::int64_t second = -1;
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x01}), &first), DBStatus::kOk);
  ASSERT_EQ(db.DeleteAccount(first), DBStatus::kOk);
  ASSERT_EQ(db.AddAccount(MakeAccount(std::byte{0x02}), &second),
            DBStatus::kOk);
  EXPECT_NE(first, second);
  EXPECT_EQ(db.GetAccount(first).error(), DBStatus::kNotFound);
}
