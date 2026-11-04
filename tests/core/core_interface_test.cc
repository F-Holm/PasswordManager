#include "core/core_interface.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <string>

namespace {

std::array<std::uint8_t, 32> MakeKey(std::uint8_t marker) {
  std::array<std::uint8_t, 32> key{};
  key.fill(marker);
  return key;
}

CoreAccount MakeAccount(const char* description, const char* email,
                        const char* username, const char* password,
                        const char* extra) {
  CoreAccount account{};
  std::strncpy(account.description, description,
               sizeof(account.description) - 1);
  std::strncpy(account.email, email, sizeof(account.email) - 1);
  std::strncpy(account.username, username, sizeof(account.username) - 1);
  std::strncpy(account.password, password, sizeof(account.password) - 1);
  std::strncpy(account.extra, extra, sizeof(account.extra) - 1);
  return account;
}

}  // namespace

class CoreInterfaceTest : public ::testing::Test {
 protected:
  const std::string test_file =
      std::string("core_interface_test_") +
      ::testing::UnitTest::GetInstance()->current_test_info()->name() +
      ".sqlite3";
  const std::array<std::uint8_t, 32> key = MakeKey(0x42);

  void SetUp() override {
    std::remove(test_file.c_str());
  }

  void TearDown() override {
    std::remove(test_file.c_str());
  }
};

TEST_F(CoreInterfaceTest, FieldSizesMatchAccountLayout) {
  EXPECT_EQ(kCoreMasterKeySize, 32u);
  EXPECT_GT(kCoreDescriptionSize, 0u);
  EXPECT_GT(kCoreEmailSize, 0u);
  EXPECT_GT(kCoreUsernameSize, 0u);
  EXPECT_GT(kCorePasswordSize, 0u);
  EXPECT_GT(kCoreExtraSize, 0u);
}

TEST_F(CoreInterfaceTest, OpenRejectsWrongKeySize) {
  const std::array<std::uint8_t, 4> short_key{1, 2, 3, 4};
  CoreSession* session =
      CoreOpen(test_file.c_str(), short_key.data(), short_key.size());
  EXPECT_EQ(session, nullptr);
}

TEST_F(CoreInterfaceTest, OpenRejectsNullArguments) {
  EXPECT_EQ(CoreOpen(nullptr, key.data(), key.size()), nullptr);
  EXPECT_EQ(CoreOpen(test_file.c_str(), nullptr, key.size()), nullptr);
}

TEST_F(CoreInterfaceTest, OpenAndCloseRoundTrip) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);
  EXPECT_TRUE(CoreIsOpen(session));

  CoreClose(session);
}

TEST_F(CoreInterfaceTest, CloseAcceptsNullptr) {
  CoreClose(nullptr);
}

TEST_F(CoreInterfaceTest, NullSessionCallsFailSafely) {
  EXPECT_FALSE(CoreIsOpen(nullptr));
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  EXPECT_FALSE(CoreAddAccount(nullptr, &account));
  EXPECT_FALSE(CoreEditAccount(nullptr, 0, &account));
  EXPECT_FALSE(CoreRemoveAccount(nullptr, 0));
  EXPECT_FALSE(CoreReorderAccount(nullptr, 0, 1));
  EXPECT_EQ(CoreLoadIndex(nullptr), 0u);
  CoreAccountEntry entry{};
  EXPECT_FALSE(CoreGetIndexEntry(nullptr, 0, &entry));
  CoreAccount out{};
  EXPECT_FALSE(CoreGetAccount(nullptr, 0, &out));
  CoreClearIndex(nullptr);
  CoreClearAccount(nullptr);
}

TEST_F(CoreInterfaceTest, AddAndGetAccountRoundTrip) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);

  const CoreAccount account =
      MakeAccount("My Bank", "me@example.com", "me", "s3cr3t", "notes");
  ASSERT_TRUE(CoreAddAccount(session, &account));

  CoreAccount fetched{};
  ASSERT_TRUE(CoreGetAccount(session, 0, &fetched));
  EXPECT_STREQ(fetched.description, "My Bank");
  EXPECT_STREQ(fetched.password, "s3cr3t");

  CoreClose(session);
}

TEST_F(CoreInterfaceTest, GetAccountFailsForMissingPosition) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);

  CoreAccount fetched{};
  EXPECT_FALSE(CoreGetAccount(session, 0, &fetched));

  CoreClose(session);
}

TEST_F(CoreInterfaceTest, EditAndRemoveAccount) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);

  const CoreAccount original = MakeAccount("old", "e", "u", "p", "x");
  ASSERT_TRUE(CoreAddAccount(session, &original));

  const CoreAccount edited = MakeAccount("new", "e", "u", "p", "x");
  ASSERT_TRUE(CoreEditAccount(session, 0, &edited));

  CoreAccount fetched{};
  ASSERT_TRUE(CoreGetAccount(session, 0, &fetched));
  EXPECT_STREQ(fetched.description, "new");

  ASSERT_TRUE(CoreRemoveAccount(session, 0));
  EXPECT_FALSE(CoreGetAccount(session, 0, &fetched));

  CoreClose(session);
}

TEST_F(CoreInterfaceTest, ReorderAccounts) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);

  const CoreAccount a = MakeAccount("a", "e", "u", "p", "x");
  const CoreAccount b = MakeAccount("b", "e", "u", "p", "x");
  ASSERT_TRUE(CoreAddAccount(session, &a));
  ASSERT_TRUE(CoreAddAccount(session, &b));

  ASSERT_TRUE(CoreReorderAccount(session, 0, 1));

  CoreAccount fetched{};
  ASSERT_TRUE(CoreGetAccount(session, 0, &fetched));
  EXPECT_STREQ(fetched.description, "b");

  CoreClose(session);
}

TEST_F(CoreInterfaceTest, LoadIndexAndReadEntries) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);

  const CoreAccount alpha = MakeAccount("alpha", "e", "u", "p", "x");
  const CoreAccount beta = MakeAccount("beta", "e", "u", "p", "x");
  ASSERT_TRUE(CoreAddAccount(session, &alpha));
  ASSERT_TRUE(CoreAddAccount(session, &beta));

  EXPECT_EQ(CoreLoadIndex(session), 2u);

  CoreAccountEntry entry{};
  ASSERT_TRUE(CoreGetIndexEntry(session, 0, &entry));
  EXPECT_EQ(entry.position, 0);
  EXPECT_STREQ(entry.description, "alpha");

  ASSERT_TRUE(CoreGetIndexEntry(session, 1, &entry));
  EXPECT_EQ(entry.position, 1);
  EXPECT_STREQ(entry.description, "beta");

  EXPECT_FALSE(CoreGetIndexEntry(session, 2, &entry));

  CoreClearIndex(session);
  EXPECT_FALSE(CoreGetIndexEntry(session, 0, &entry));

  CoreClose(session);
}

TEST_F(CoreInterfaceTest, ClearAccountDropsLoadedAccount) {
  CoreSession* session = CoreOpen(test_file.c_str(), key.data(), key.size());
  ASSERT_NE(session, nullptr);

  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  ASSERT_TRUE(CoreAddAccount(session, &account));

  CoreAccount fetched{};
  ASSERT_TRUE(CoreGetAccount(session, 0, &fetched));

  CoreClearAccount(session);

  CoreClose(session);
}
