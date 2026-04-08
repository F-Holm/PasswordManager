#include "storage/storage.h"

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

class StorageTest : public ::testing::Test {
 protected:
  const std::string test_file = "test_database.bin";

  void SetUp() override {
    std::ofstream ofs(test_file, std::ios::binary | std::ios::trunc);
    ofs.close();
  }

  void TearDown() override { std::remove(test_file.c_str()); }
};

// --- Connection Tests ---

TEST_F(StorageTest, FileOpensCorrectly) {
  Storage<Record> Storage(test_file);
  EXPECT_TRUE(Storage.IsOpen());
}

// --- Write and Read Tests ---

TEST_F(StorageTest, AddAndReadSequential) {
  Storage<Record> Storage(test_file);

  Record r1 = {1, "Alice"};
  Record r2 = {2, "Bob"};

  // Write records
  Storage.element = r1;
  ASSERT_TRUE(Storage.Add());

  Storage.element = r2;
  ASSERT_TRUE(Storage.Add());

  // Read back position 0
  ASSERT_TRUE(Storage.ReadPos(0));
  EXPECT_EQ(Storage.element.id, 1);
  EXPECT_STREQ(Storage.element.name, "Alice");

  // Read back position 1
  ASSERT_TRUE(Storage.ReadPos(1));
  EXPECT_EQ(Storage.element.id, 2);
  EXPECT_STREQ(Storage.element.name, "Bob");
}

// --- Update Tests ---

TEST_F(StorageTest, UpdateExistingRecord) {
  Storage<Record> Storage(test_file);

  Storage.element = {10, "Original"};
  Storage.Add();

  // Modify record at pos 0
  Storage.element = {10, "Updated"};
  EXPECT_TRUE(Storage.UpdatePos(0));

  // Verify change
  Storage.element = {0, ""};  // Reset local element
  Storage.ReadPos(0);
  EXPECT_STREQ(Storage.element.name, "Updated");
}

// --- Deletion (Clearing) Tests ---

TEST_F(StorageTest, DeleteRecordAtPosition) {
  Storage<Record> Storage(test_file);

  Storage.element = {99, "To Delete"};
  Storage.Add();

  // DeletePos calls Clear() and then writes it
  EXPECT_TRUE(Storage.DeletePos(0));

  // Verify the record is "cleared" (assuming SecureClear zeros it out)
  Storage.ReadPos(0);
  EXPECT_EQ(Storage.element.id, 0);
  EXPECT_STREQ(Storage.element.name, "");
}

// --- Edge Cases and Persistence ---

TEST_F(StorageTest, ReadEmptyFileReturnsFalse) {
  Storage<Record> Storage(test_file);
  // Should fail as there is no record at index 0
  EXPECT_FALSE(Storage.ReadPos(0));
}

TEST_F(StorageTest, PersistenceBetweenSessions) {
  // Session 1: Write data
  {
    Storage<Record> Storage(test_file);
    Storage.element = {42, "Deep Thought"};
    Storage.Add();
  }  // File closes here

  // Session 2: Read data
  {
    Storage<Record> Storage(test_file);
    EXPECT_TRUE(Storage.ReadPos(0));
    EXPECT_EQ(Storage.element.id, 42);
  }
}
