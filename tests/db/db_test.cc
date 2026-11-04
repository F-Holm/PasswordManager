#include "db/db.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "types/account_binary.h"

namespace {

AccountBinary MakeAccount(std::byte marker) {
  AccountBinary account;
  account.description.fill(marker);
  account.data.fill(marker);
  account.iv_description.fill(marker);
  account.iv_data.fill(marker);
  account.tag_description.fill(marker);
  account.tag_data.fill(marker);
  return account;
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
  EXPECT_EQ(db.Count(), 0u);
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

  EXPECT_FALSE(db.AddAccount(MakeAccount(std::byte{0x01})));
  EXPECT_FALSE(db.GetAccount(0).has_value());
  EXPECT_FALSE(db.UpdateAccount(0, MakeAccount(std::byte{0x01})));
  EXPECT_FALSE(db.DeleteAccount(0));
  EXPECT_FALSE(db.MoveAccount(0, 1));
  EXPECT_TRUE(db.GetAllIndex().empty());
  EXPECT_EQ(db.Count(), 0u);
  EXPECT_FALSE(db.Compact());
}

TEST_F(DBTest, AddAndGetAccount) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  AccountBinary account = MakeAccount(std::byte{0xAB});
  ASSERT_TRUE(db.AddAccount(account));
  EXPECT_EQ(db.Count(), 1u);

  auto fetched = db.GetAccount(0);
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(std::memcmp(fetched->description.data(), account.description.data(),
                        account.description.size()),
            0);
  EXPECT_EQ(std::memcmp(fetched->data.data(), account.data.data(),
                        account.data.size()),
            0);
}

TEST_F(DBTest, GetMissingAccountReturnsNullopt) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_FALSE(db.GetAccount(0).has_value());
}

TEST_F(DBTest, GetAccountReturnsNulloptWhenBlobIsCorrupted) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));

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

  EXPECT_FALSE(db.GetAccount(0).has_value());
}

TEST_F(DBTest, AddAppendsAtIncrementingPositions) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x03})));

  ASSERT_TRUE(db.GetAccount(0).has_value());
  ASSERT_TRUE(db.GetAccount(1).has_value());
  ASSERT_TRUE(db.GetAccount(2).has_value());
  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(1)->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(2)->description[0], std::byte{0x03});
}

TEST_F(DBTest, UpdateAccountOverwritesExistingRecord) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.UpdateAccount(0, MakeAccount(std::byte{0x02})));

  auto fetched = db.GetAccount(0);
  ASSERT_TRUE(fetched.has_value());
  EXPECT_EQ(fetched->description[0], std::byte{0x02});
}

TEST_F(DBTest, UpdateMissingAccountFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_FALSE(db.UpdateAccount(0, MakeAccount(std::byte{0x02})));
}

TEST_F(DBTest, DeleteAccountShiftsFollowingPositions) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x03})));

  ASSERT_TRUE(db.DeleteAccount(0));

  EXPECT_EQ(db.Count(), 2u);
  ASSERT_TRUE(db.GetAccount(0).has_value());
  ASSERT_TRUE(db.GetAccount(1).has_value());
  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(1)->description[0], std::byte{0x03});
  EXPECT_FALSE(db.GetAccount(2).has_value());
}

TEST_F(DBTest, DeleteMissingAccountFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_FALSE(db.DeleteAccount(0));
}

TEST_F(DBTest, MoveAccountForward) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x03})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x04})));

  // Move item at 0 to 2: expected order becomes 2, 3, 1, 4
  ASSERT_TRUE(db.MoveAccount(0, 2));

  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(1)->description[0], std::byte{0x03});
  EXPECT_EQ(db.GetAccount(2)->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(3)->description[0], std::byte{0x04});
}

TEST_F(DBTest, MoveAccountBackward) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x03})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x04})));

  // Move item at 3 to 1: expected order becomes 1, 4, 2, 3
  ASSERT_TRUE(db.MoveAccount(3, 1));

  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(1)->description[0], std::byte{0x04});
  EXPECT_EQ(db.GetAccount(2)->description[0], std::byte{0x02});
  EXPECT_EQ(db.GetAccount(3)->description[0], std::byte{0x03});
}

TEST_F(DBTest, MoveAccountToSamePositionIsNoop) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));

  EXPECT_TRUE(db.MoveAccount(1, 1));

  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(1)->description[0], std::byte{0x02});
}

TEST_F(DBTest, MoveAccountOutOfRangeFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));

  EXPECT_FALSE(db.MoveAccount(0, 5));
  EXPECT_FALSE(db.MoveAccount(-1, 0));
}

TEST_F(DBTest, MoveAccountFromMissingPositionFails) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));

  EXPECT_FALSE(db.MoveAccount(50, 0));
}

TEST_F(DBTest, GetAllIndexReturnsOrderedEntries) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x03})));

  auto index = db.GetAllIndex();
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

  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));
  ASSERT_TRUE(db.DeleteAccount(0));

  EXPECT_TRUE(db.Compact());
  EXPECT_EQ(db.Count(), 1u);
  ASSERT_TRUE(db.GetAccount(0).has_value());
  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x02});
}

TEST_F(DBTest, OperationsFailGracefullyIfSchemaIsCorrupted) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));

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

  EXPECT_FALSE(db.AddAccount(MakeAccount(std::byte{0x02})));
  EXPECT_FALSE(db.GetAccount(0).has_value());
  EXPECT_FALSE(db.UpdateAccount(0, MakeAccount(std::byte{0x02})));
  EXPECT_FALSE(db.DeleteAccount(0));
  EXPECT_FALSE(db.MoveAccount(0, 1));
  EXPECT_TRUE(db.GetAllIndex().empty());
  EXPECT_EQ(db.Count(), 0u);
}

TEST_F(DBTest, PersistenceBetweenSessions) {
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x2A})));
  }

  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    auto fetched = db.GetAccount(0);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched->description[0], std::byte{0x2A});
  }
}

TEST_F(DBTest, WritesFailCleanlyWhileAnotherConnectionHoldsTheWriteLock) {
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x01})));
  ASSERT_TRUE(db.AddAccount(MakeAccount(std::byte{0x02})));

  sqlite3* other = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &other), SQLITE_OK);
  sqlite3_busy_timeout(other, 0);
  ASSERT_EQ(sqlite3_exec(other, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr),
            SQLITE_OK);

  EXPECT_FALSE(db.AddAccount(MakeAccount(std::byte{0x03})));
  EXPECT_FALSE(db.DeleteAccount(0));
  EXPECT_FALSE(db.MoveAccount(0, 1));

  sqlite3_exec(other, "ROLLBACK;", nullptr, nullptr, nullptr);
  sqlite3_close(other);

  // Nothing was half-applied: both accounts are still at their positions.
  ASSERT_EQ(db.Count(), 2u);
  EXPECT_EQ(db.GetAccount(0)->description[0], std::byte{0x01});
  EXPECT_EQ(db.GetAccount(1)->description[0], std::byte{0x02});
}
