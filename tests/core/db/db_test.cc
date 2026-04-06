#include "db/db.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>

struct Record {
  int id;
  char name[32];

  bool operator==(const Record& other) const {
    return id == other.id && std::string(name) == std::string(other.name);
  }
};

class DBTest : public ::testing::Test {
 protected:
  const std::string test_file = "test_database.bin";

  void SetUp() override {
    std::ofstream ofs(test_file, std::ios::binary | std::ios::trunc);
    ofs.close();
  }

  void TearDown() override { std::remove(test_file.c_str()); }
};

// --- Connection Tests ---

TEST_F(DBTest, FileOpensCorrectly) {
  DB<Record> db(test_file);
  EXPECT_TRUE(db.IsOpen());
}

// --- Write and Read Tests ---

TEST_F(DBTest, AddAndReadSequential) {
  DB<Record> db(test_file);

  Record r1 = {1, "Alice"};
  Record r2 = {2, "Bob"};

  // Write records
  db.element = r1;
  ASSERT_TRUE(db.Add());

  db.element = r2;
  ASSERT_TRUE(db.Add());

  // Read back position 0
  ASSERT_TRUE(db.ReadPos(0));
  EXPECT_EQ(db.element.id, 1);
  EXPECT_STREQ(db.element.name, "Alice");

  // Read back position 1
  ASSERT_TRUE(db.ReadPos(1));
  EXPECT_EQ(db.element.id, 2);
  EXPECT_STREQ(db.element.name, "Bob");
}

// --- Update Tests ---

TEST_F(DBTest, UpdateExistingRecord) {
  DB<Record> db(test_file);

  db.element = {10, "Original"};
  db.Add();

  // Modify record at pos 0
  db.element = {10, "Updated"};
  EXPECT_TRUE(db.UpdatePos(0));

  // Verify change
  db.element = {0, ""};  // Reset local element
  db.ReadPos(0);
  EXPECT_STREQ(db.element.name, "Updated");
}

// --- Deletion (Clearing) Tests ---

TEST_F(DBTest, DeleteRecordAtPosition) {
  DB<Record> db(test_file);

  db.element = {99, "To Delete"};
  db.Add();

  // DeletePos calls Clear() and then writes it
  EXPECT_TRUE(db.DeletePos(0));

  // Verify the record is "cleared" (assuming SecureClear zeros it out)
  db.ReadPos(0);
  EXPECT_EQ(db.element.id, 0);
  EXPECT_STREQ(db.element.name, "");
}

// --- Edge Cases and Persistence ---

TEST_F(DBTest, ReadEmptyFileReturnsFalse) {
  DB<Record> db(test_file);
  // Should fail as there is no record at index 0
  EXPECT_FALSE(db.ReadPos(0));
}

TEST_F(DBTest, PersistenceBetweenSessions) {
  // Session 1: Write data
  {
    DB<Record> db(test_file);
    db.element = {42, "Deep Thought"};
    db.Add();
  }  // File closes here

  // Session 2: Read data
  {
    DB<Record> db(test_file);
    EXPECT_TRUE(db.ReadPos(0));
    EXPECT_EQ(db.element.id, 42);
  }
}
