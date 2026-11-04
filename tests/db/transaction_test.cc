#include "db/transaction.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <cstdint>
#include <cstdio>
#include <string>

namespace {

std::int64_t CountRows(sqlite3* db) {
  sqlite3_stmt* stmt = nullptr;
  sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t;", -1, &stmt, nullptr);
  sqlite3_step(stmt);
  const std::int64_t count = sqlite3_column_int64(stmt, 0);
  sqlite3_finalize(stmt);
  return count;
}

class TransactionTest : public ::testing::Test {
 protected:
  const std::string test_file =
      std::string("test_transaction_") +
      ::testing::UnitTest::GetInstance()->current_test_info()->name() +
      ".sqlite3";
  sqlite3* db = nullptr;

  void SetUp() override {
    std::remove(test_file.c_str());
    ASSERT_EQ(sqlite3_open(test_file.c_str(), &db), SQLITE_OK);
    ASSERT_EQ(sqlite3_exec(db, "CREATE TABLE t (v INTEGER);", nullptr, nullptr,
                           nullptr),
              SQLITE_OK);
  }

  void TearDown() override {
    sqlite3_close(db);
    std::remove(test_file.c_str());
  }

  void Insert() const {
    ASSERT_EQ(sqlite3_exec(db, "INSERT INTO t VALUES (1);", nullptr, nullptr,
                           nullptr),
              SQLITE_OK);
  }
};

}  // namespace

TEST_F(TransactionTest, BeginsOnFreeDatabase) {
  const Transaction tx(db);
  EXPECT_TRUE(tx.IsActive());
  EXPECT_EQ(tx.BeginResult(), SQLITE_OK);
}

TEST_F(TransactionTest, NullDatabaseIsNotActive) {
  Transaction tx(nullptr);
  EXPECT_FALSE(tx.IsActive());
  EXPECT_FALSE(tx.Commit());
}

TEST_F(TransactionTest, CommitPersistsChanges) {
  {
    Transaction tx(db);
    ASSERT_TRUE(tx.IsActive());
    Insert();
    EXPECT_TRUE(tx.Commit());
    EXPECT_FALSE(tx.IsActive());
  }
  EXPECT_EQ(CountRows(db), 1);
}

TEST_F(TransactionTest, DestructorRollsBackUncommittedChanges) {
  {
    const Transaction tx(db);
    ASSERT_TRUE(tx.IsActive());
    Insert();
  }
  EXPECT_EQ(CountRows(db), 0);
}

TEST_F(TransactionTest, SecondCommitFails) {
  Transaction tx(db);
  ASSERT_TRUE(tx.Commit());
  EXPECT_FALSE(tx.Commit());
}

TEST_F(TransactionTest, ReportsBusyWhenAnotherConnectionHoldsTheWriteLock) {
  sqlite3* other = nullptr;
  ASSERT_EQ(sqlite3_open(test_file.c_str(), &other), SQLITE_OK);
  ASSERT_EQ(sqlite3_exec(other, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr),
            SQLITE_OK);

  {
    const Transaction tx(db);
    EXPECT_FALSE(tx.IsActive());
    EXPECT_EQ(tx.BeginResult(), SQLITE_BUSY);
  }

  sqlite3_exec(other, "ROLLBACK;", nullptr, nullptr, nullptr);
  sqlite3_close(other);

  const Transaction tx(db);
  EXPECT_TRUE(tx.IsActive());
}

TEST_F(TransactionTest, NestedGuardJoinsTheOuterTransaction) {
  {
    Transaction outer(db);
    ASSERT_TRUE(outer.IsActive());
    EXPECT_FALSE(outer.IsNested());
    {
      Transaction inner(db);
      EXPECT_TRUE(inner.IsActive());
      EXPECT_TRUE(inner.IsNested());
      Insert();
      EXPECT_TRUE(inner.Commit());
    }
    // The inner commit did not commit: the outer one is still open...
    EXPECT_EQ(sqlite3_get_autocommit(db), 0);
  }
  // ...and rolling it back undoes the inner work too.
  EXPECT_EQ(CountRows(db), 0);
}

TEST_F(TransactionTest, NestedGuardDoesNotRollBackTheOuterTransaction) {
  {
    Transaction outer(db);
    Insert();
    {
      const Transaction inner(db);
      ASSERT_TRUE(inner.IsNested());
    }
    EXPECT_TRUE(outer.Commit());
  }
  EXPECT_EQ(CountRows(db), 1);
}
