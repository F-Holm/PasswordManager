#include "core/core_interface.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <string>

#include "crypto/crypto.h"
#include "types/account_size.h"

extern "C" int CoreCHeaderSmokeTest(void);

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
  CoreSession* session = nullptr;

  void SetUp() override {
    std::remove(test_file.c_str());
    session = CoreNew();
    ASSERT_NE(session, nullptr);
  }

  void TearDown() override {
    CoreFree(session);
    std::remove(test_file.c_str());
  }

  CoreResult Open() const {
    return CoreOpenWithKey(session, test_file.c_str(), key.data(), key.size());
  }
};

TEST_F(CoreInterfaceTest, FieldSizesMatchAccountLayout) {
  EXPECT_EQ(sizeof(CoreAccount::description), AccountSize::kDescription);
  EXPECT_EQ(sizeof(CoreAccount::email), AccountSize::kEmail);
  EXPECT_EQ(sizeof(CoreAccount::username), AccountSize::kUsername);
  EXPECT_EQ(sizeof(CoreAccount::password), AccountSize::kPassword);
  EXPECT_EQ(sizeof(CoreAccount::extra), AccountSize::kExtra);
  EXPECT_EQ(static_cast<std::size_t>(kCoreMasterKeySize), Crypto::Size::kKey);
}

TEST_F(CoreInterfaceTest, HeaderCompilesAndWorksFromC) {
  EXPECT_EQ(CoreCHeaderSmokeTest(), 0);
}

TEST_F(CoreInterfaceTest, ResultNames) {
  EXPECT_STREQ(CoreResultName(kCoreOk), "ok");
  EXPECT_STREQ(CoreResultName(kCoreBusy), "busy");
  EXPECT_STREQ(CoreResultName(kCoreCorrupted), "corrupted");
  EXPECT_STREQ(CoreResultName(12345), "unknown");
}

TEST_F(CoreInterfaceTest, NewSessionIsNotOpen) {
  EXPECT_FALSE(CoreIsOpen(session));
}

TEST_F(CoreInterfaceTest, OpenRejectsWrongKeySize) {
  const std::array<std::uint8_t, 4> short_key{1, 2, 3, 4};
  EXPECT_EQ(CoreOpenWithKey(session, test_file.c_str(), short_key.data(),
                            short_key.size()),
            kCoreInvalidArgument);
  EXPECT_FALSE(CoreIsOpen(session));
}

TEST_F(CoreInterfaceTest, OpenRejectsNullArguments) {
  EXPECT_EQ(CoreOpenWithKey(nullptr, test_file.c_str(), key.data(), key.size()),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreOpenWithKey(session, nullptr, key.data(), key.size()),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreOpenWithKey(session, test_file.c_str(), nullptr, key.size()),
            kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, OpenFailsForUnreachablePath) {
  EXPECT_EQ(CoreOpenWithKey(session, "/nonexistent_dir_xyz/vault.sqlite3",
                            key.data(), key.size()),
            kCoreError);
}

TEST_F(CoreInterfaceTest, OpenAndCloseRoundTrip) {
  ASSERT_EQ(Open(), kCoreOk);
  EXPECT_TRUE(CoreIsOpen(session));

  CoreClose(session);
  EXPECT_FALSE(CoreIsOpen(session));

  // The session can be reused after closing.
  ASSERT_EQ(Open(), kCoreOk);
  EXPECT_TRUE(CoreIsOpen(session));
}

TEST_F(CoreInterfaceTest, NullSessionCallsFailSafely) {
  CoreFree(nullptr);
  CoreClose(nullptr);
  EXPECT_FALSE(CoreIsOpen(nullptr));
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  EXPECT_EQ(CoreAddAccount(nullptr, &account), kCoreInvalidArgument);
  EXPECT_EQ(CoreEditAccount(nullptr, 0, &account), kCoreInvalidArgument);
  EXPECT_EQ(CoreRemoveAccount(nullptr, 0), kCoreInvalidArgument);
  EXPECT_EQ(CoreReorderAccount(nullptr, 0, 1), kCoreInvalidArgument);
  std::size_t count = 0;
  EXPECT_EQ(CoreLoadIndex(nullptr, &count), kCoreInvalidArgument);
  CoreAccountEntry entry{};
  EXPECT_EQ(CoreGetIndexEntry(nullptr, 0, &entry), kCoreInvalidArgument);
  CoreAccount out{};
  EXPECT_EQ(CoreGetAccount(nullptr, 0, &out), kCoreInvalidArgument);
  CoreClearIndex(nullptr);
  CoreClearAccount(nullptr);
}

TEST_F(CoreInterfaceTest, NullOutputPointersAreRejected) {
  ASSERT_EQ(Open(), kCoreOk);
  EXPECT_EQ(CoreAddAccount(session, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreEditAccount(session, 0, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreLoadIndex(session, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreGetIndexEntry(session, 0, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreGetAccount(session, 0, nullptr), kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, OperationsReportLockedWhenNotOpen) {
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  EXPECT_EQ(CoreAddAccount(session, &account), kCoreLocked);
  std::size_t count = 0;
  EXPECT_EQ(CoreLoadIndex(session, &count), kCoreLocked);
  CoreAccount out{};
  EXPECT_EQ(CoreGetAccount(session, 0, &out), kCoreLocked);
}

TEST_F(CoreInterfaceTest, AddAndGetAccountRoundTrip) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount account =
      MakeAccount("My Bank", "me@example.com", "me", "s3cr3t", "notes");
  ASSERT_EQ(CoreAddAccount(session, &account), kCoreOk);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, 0, &fetched), kCoreOk);
  EXPECT_STREQ(fetched.description, "My Bank");
  EXPECT_STREQ(fetched.email, "me@example.com");
  EXPECT_STREQ(fetched.username, "me");
  EXPECT_STREQ(fetched.password, "s3cr3t");
  EXPECT_STREQ(fetched.extra, "notes");
}

TEST_F(CoreInterfaceTest, GetAccountReportsNotFoundForMissingPosition) {
  ASSERT_EQ(Open(), kCoreOk);

  CoreAccount fetched{};
  EXPECT_EQ(CoreGetAccount(session, 0, &fetched), kCoreNotFound);
}

TEST_F(CoreInterfaceTest, EditAndRemoveAccount) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount original = MakeAccount("old", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &original), kCoreOk);

  const CoreAccount edited = MakeAccount("new", "e", "u", "p", "x");
  ASSERT_EQ(CoreEditAccount(session, 0, &edited), kCoreOk);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, 0, &fetched), kCoreOk);
  EXPECT_STREQ(fetched.description, "new");

  ASSERT_EQ(CoreRemoveAccount(session, 0), kCoreOk);
  EXPECT_EQ(CoreGetAccount(session, 0, &fetched), kCoreNotFound);
  EXPECT_EQ(CoreRemoveAccount(session, 0), kCoreNotFound);
}

TEST_F(CoreInterfaceTest, ReorderAccounts) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount a = MakeAccount("a", "e", "u", "p", "x");
  const CoreAccount b = MakeAccount("b", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &a), kCoreOk);
  ASSERT_EQ(CoreAddAccount(session, &b), kCoreOk);

  ASSERT_EQ(CoreReorderAccount(session, 0, 1), kCoreOk);
  EXPECT_EQ(CoreReorderAccount(session, 0, 7), kCoreNotFound);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, 0, &fetched), kCoreOk);
  EXPECT_STREQ(fetched.description, "b");
}

TEST_F(CoreInterfaceTest, LoadIndexAndReadEntries) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount alpha = MakeAccount("alpha", "e", "u", "p", "x");
  const CoreAccount beta = MakeAccount("beta", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &alpha), kCoreOk);
  ASSERT_EQ(CoreAddAccount(session, &beta), kCoreOk);

  std::size_t count = 0;
  ASSERT_EQ(CoreLoadIndex(session, &count), kCoreOk);
  EXPECT_EQ(count, 2u);

  CoreAccountEntry entry{};
  ASSERT_EQ(CoreGetIndexEntry(session, 0, &entry), kCoreOk);
  EXPECT_EQ(entry.position, 0);
  EXPECT_STREQ(entry.description, "alpha");

  ASSERT_EQ(CoreGetIndexEntry(session, 1, &entry), kCoreOk);
  EXPECT_EQ(entry.position, 1);
  EXPECT_STREQ(entry.description, "beta");

  EXPECT_EQ(CoreGetIndexEntry(session, 2, &entry), kCoreNotFound);

  CoreClearIndex(session);
  EXPECT_EQ(CoreGetIndexEntry(session, 0, &entry), kCoreNotFound);
}

TEST_F(CoreInterfaceTest, ClearAccountDropsLoadedAccount) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &account), kCoreOk);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, 0, &fetched), kCoreOk);

  CoreClearAccount(session);
}
