#include "db/storage.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>

#include "types/account_index.h"

class StorageTest : public ::testing::Test {
 protected:
  const std::string test_file = "test_database.bin";

  void SetUp() override {
    std::ofstream ofs(test_file, std::ios::binary | std::ios::trunc);
    ofs.close();
  }

  void TearDown() override {
    std::remove(test_file.c_str());
  }
};

template <typename... Args>
AccountIndex CreateElement(Args... bytes) {
  AccountIndex element;

  std::byte array_args[] = {bytes...};

  for (size_t i = 0; i < sizeof...(bytes); i++)
    element.description[i] = array_args[i];
  return element;
}

// --- Connection Tests ---

TEST_F(StorageTest, FileOpensCorrectly) {
  Storage<AccountIndex> Storage(test_file);
  EXPECT_TRUE(Storage.IsOpen());
}

// --- Write and Read Tests ---

TEST_F(StorageTest, AddAndReadSequential) {
  Storage<AccountIndex> Storage(test_file);

  AccountIndex r1 = CreateElement(std::byte{0xDE}, std::byte{0xAD});
  AccountIndex r2 = CreateElement(std::byte{0xBE}, std::byte{0xEF});

  // Write records
  Storage.element = r1;
  ASSERT_TRUE(Storage.Add());

  Storage.element = r2;
  ASSERT_TRUE(Storage.Add());

  // Read back position 0
  ASSERT_TRUE(Storage.ReadPos(0));
  EXPECT_EQ(Storage.element.description[0], std::byte{0xDE});
  EXPECT_EQ(Storage.element.description[1], std::byte{0xAD});

  // Read back position 1
  ASSERT_TRUE(Storage.ReadPos(1));
  EXPECT_EQ(Storage.element.description[0], std::byte{0xBE});
  EXPECT_EQ(Storage.element.description[1], std::byte{0xEF});
}

// --- Update Tests ---

TEST_F(StorageTest, UpdateExistingRecord) {
  Storage<AccountIndex> Storage(test_file);

  Storage.element = CreateElement(std::byte{0xDE}, std::byte{0xAD});
  Storage.Add();

  // Modify record at pos 0
  Storage.element = CreateElement(std::byte{0xBE}, std::byte{0xEF});
  EXPECT_TRUE(Storage.UpdatePos(0));

  // Verify change
  Storage.element = CreateElement(std::byte{0x00});
  Storage.ReadPos(0);
  EXPECT_EQ(Storage.element.description[0], std::byte{0xBE});
}

// --- Deletion (Clearing) Tests ---

TEST_F(StorageTest, DeleteRecordAtPosition) {
  Storage<AccountIndex> Storage(test_file);

  Storage.element = CreateElement(std::byte{0xDE}, std::byte{0xAD});
  Storage.Add();

  // DeletePos calls Clear() and then writes it
  EXPECT_TRUE(Storage.DeletePos(0));

  // Verify the record is "cleared" (assuming SecureClear zeros it out)
  Storage.ReadPos(0);
  EXPECT_EQ(Storage.element.description[0], std::byte{0x00});
  EXPECT_EQ(Storage.element.description[1], std::byte{0x00});
}

// --- Edge Cases and Persistence ---

TEST_F(StorageTest, ReadEmptyFileReturnsFalse) {
  Storage<AccountIndex> Storage(test_file);
  // Should fail as there is no record at index 0
  EXPECT_FALSE(Storage.ReadPos(0));
}

TEST_F(StorageTest, PersistenceBetweenSessions) {
  // Session 1: Write data
  {
    Storage<AccountIndex> Storage(test_file);
    Storage.element = CreateElement(std::byte{0xDE}, std::byte{0xAD});
    Storage.Add();
  }  // File closes here

  // Session 2: Read data
  {
    Storage<AccountIndex> Storage(test_file);
    EXPECT_TRUE(Storage.ReadPos(0));
    EXPECT_EQ(Storage.element.description[0], std::byte{0xDE});
    EXPECT_EQ(Storage.element.description[1], std::byte{0xAD});
  }
}
