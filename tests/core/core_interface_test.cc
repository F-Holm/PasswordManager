#include "core/core_interface.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <string>

#include "crypto/crypto.h"
#include "db/db.h"
#include "types/account_size.h"

extern "C" int CoreCHeaderSmokeTest(void);

namespace {

constexpr char kPassword[] = "correct horse battery staple";
constexpr std::size_t kPasswordSize = sizeof(kPassword) - 1;
constexpr CoreKdfParams kTestKdf{
    .memory_kib = 8, .iterations = 1, .parallelism = 1};

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

  // Id of the account at `position` in the vault file, or -1 if there is
  // none, read through a separate connection so it has no effect on the
  // Core under test. Lets the tests speak in positions.
  [[nodiscard]] std::int64_t IdAt(std::int64_t position) const {
    DB db;
    if (!db.Open(test_file, DB::OpenMode::kExistingOnly)) {
      return -1;
    }
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

  CoreResult Open() const {
    return CoreCreate(session, test_file.c_str(), kPassword, kPasswordSize,
                      &kTestKdf);
  }
};

TEST_F(CoreInterfaceTest, FieldSizesMatchAccountLayout) {
  EXPECT_EQ(sizeof(CoreAccount::description), AccountSize::kDescription);
  EXPECT_EQ(sizeof(CoreAccount::email), AccountSize::kEmail);
  EXPECT_EQ(sizeof(CoreAccount::username), AccountSize::kUsername);
  EXPECT_EQ(sizeof(CoreAccount::password), AccountSize::kPassword);
  EXPECT_EQ(sizeof(CoreAccount::extra), AccountSize::kExtra);
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

TEST_F(CoreInterfaceTest, DefaultKdfParamsMatchCore) {
  const CoreKdfParams params = CoreDefaultKdfParams();
  EXPECT_EQ(params.memory_kib, Crypto::kDefaultKdfParams.memory_kib);
  EXPECT_EQ(params.iterations, Crypto::kDefaultKdfParams.iterations);
  EXPECT_EQ(params.parallelism, Crypto::kDefaultKdfParams.parallelism);
}

TEST_F(CoreInterfaceTest, CreateAndUnlockRejectNullArguments) {
  const char* path = test_file.c_str();
  EXPECT_EQ(CoreCreate(nullptr, path, kPassword, kPasswordSize, &kTestKdf),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreCreate(session, nullptr, kPassword, kPasswordSize, &kTestKdf),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreCreate(session, path, nullptr, kPasswordSize, &kTestKdf),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreUnlock(nullptr, path, kPassword, kPasswordSize),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreUnlock(session, nullptr, kPassword, kPasswordSize),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreUnlock(session, path, nullptr, kPasswordSize),
            kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, CreateRejectsEmptyPasswordAndBadParams) {
  EXPECT_EQ(CoreCreate(session, test_file.c_str(), kPassword, 0, &kTestKdf),
            kCoreInvalidArgument);
  const CoreKdfParams bad{.memory_kib = 1, .iterations = 1, .parallelism = 1};
  EXPECT_EQ(
      CoreCreate(session, test_file.c_str(), kPassword, kPasswordSize, &bad),
      kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, CreateFailsForUnreachablePath) {
  EXPECT_EQ(CoreCreate(session, "/nonexistent_dir_xyz/vault.sqlite3", kPassword,
                       kPasswordSize, &kTestKdf),
            kCoreError);
}

TEST_F(CoreInterfaceTest, CreateTwiceReportsAlreadyExists) {
  ASSERT_EQ(Open(), kCoreOk);
  EXPECT_EQ(Open(), kCoreAlreadyExists);
}

TEST_F(CoreInterfaceTest, UnlockChecksThePassword) {
  ASSERT_EQ(Open(), kCoreOk);
  const CoreAccount account = MakeAccount("d", "e", "u", "s3cr3t", "x");
  ASSERT_EQ(CoreAddAccount(session, &account, nullptr), kCoreOk);
  CoreClose(session);

  EXPECT_EQ(CoreUnlock(session, test_file.c_str(), "nope", 4),
            kCoreWrongPassword);
  EXPECT_FALSE(CoreIsOpen(session));

  // The password does not need to be NUL-terminated.
  const std::string padded = std::string(kPassword) + "trailing garbage";
  ASSERT_EQ(
      CoreUnlock(session, test_file.c_str(), padded.data(), kPasswordSize),
      kCoreOk);
  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreOk);
  EXPECT_STREQ(fetched.password, "s3cr3t");
}

TEST_F(CoreInterfaceTest, UnlockMissingVaultReportsNotFound) {
  EXPECT_EQ(CoreUnlock(session, test_file.c_str(), kPassword, kPasswordSize),
            kCoreNotFound);
}

TEST_F(CoreInterfaceTest, OpenAndCloseRoundTrip) {
  ASSERT_EQ(Open(), kCoreOk);
  EXPECT_TRUE(CoreIsOpen(session));

  CoreClose(session);
  EXPECT_FALSE(CoreIsOpen(session));

  // The session can be reused after closing.
  ASSERT_EQ(CoreUnlock(session, test_file.c_str(), kPassword, kPasswordSize),
            kCoreOk);
  EXPECT_TRUE(CoreIsOpen(session));
}

TEST_F(CoreInterfaceTest, NullSessionCallsFailSafely) {
  CoreFree(nullptr);
  CoreClose(nullptr);
  EXPECT_FALSE(CoreIsOpen(nullptr));
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  EXPECT_EQ(CoreAddAccount(nullptr, &account, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreEditAccount(nullptr, 0, kCoreAnyRevision, &account),
            kCoreInvalidArgument);
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
  EXPECT_EQ(CoreAddAccount(session, nullptr, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreEditAccount(session, IdAt(0), kCoreAnyRevision, nullptr),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreLoadIndex(session, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreGetIndexEntry(session, 0, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreGetAccount(session, IdAt(0), nullptr), kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, OperationsReportLockedWhenNotOpen) {
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  EXPECT_EQ(CoreAddAccount(session, &account, nullptr), kCoreLocked);
  std::size_t count = 0;
  EXPECT_EQ(CoreLoadIndex(session, &count), kCoreLocked);
  CoreAccount out{};
  EXPECT_EQ(CoreGetAccount(session, IdAt(0), &out), kCoreLocked);
}

TEST_F(CoreInterfaceTest, AddAndGetAccountRoundTrip) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount account =
      MakeAccount("My Bank", "me@example.com", "me", "s3cr3t", "notes");
  ASSERT_EQ(CoreAddAccount(session, &account, nullptr), kCoreOk);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreOk);
  EXPECT_STREQ(fetched.description, "My Bank");
  EXPECT_STREQ(fetched.email, "me@example.com");
  EXPECT_STREQ(fetched.username, "me");
  EXPECT_STREQ(fetched.password, "s3cr3t");
  EXPECT_STREQ(fetched.extra, "notes");
}

TEST_F(CoreInterfaceTest, GetAccountReportsNotFoundForMissingPosition) {
  ASSERT_EQ(Open(), kCoreOk);

  CoreAccount fetched{};
  EXPECT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreNotFound);
}

TEST_F(CoreInterfaceTest, EditAndRemoveAccount) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount original = MakeAccount("old", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &original, nullptr), kCoreOk);

  const CoreAccount edited = MakeAccount("new", "e", "u", "p", "x");
  ASSERT_EQ(CoreEditAccount(session, IdAt(0), kCoreAnyRevision, &edited),
            kCoreOk);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreOk);
  EXPECT_STREQ(fetched.description, "new");

  ASSERT_EQ(CoreRemoveAccount(session, IdAt(0)), kCoreOk);
  EXPECT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreNotFound);
  EXPECT_EQ(CoreRemoveAccount(session, IdAt(0)), kCoreNotFound);
}

TEST_F(CoreInterfaceTest, ReorderAccounts) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount a = MakeAccount("a", "e", "u", "p", "x");
  const CoreAccount b = MakeAccount("b", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &a, nullptr), kCoreOk);
  ASSERT_EQ(CoreAddAccount(session, &b, nullptr), kCoreOk);

  ASSERT_EQ(CoreReorderAccount(session, IdAt(0), 1), kCoreOk);
  EXPECT_EQ(CoreReorderAccount(session, IdAt(0), 7), kCoreNotFound);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreOk);
  EXPECT_STREQ(fetched.description, "b");
}

TEST_F(CoreInterfaceTest, LoadIndexAndReadEntries) {
  ASSERT_EQ(Open(), kCoreOk);

  const CoreAccount alpha = MakeAccount("alpha", "e", "u", "p", "x");
  const CoreAccount beta = MakeAccount("beta", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &alpha, nullptr), kCoreOk);
  ASSERT_EQ(CoreAddAccount(session, &beta, nullptr), kCoreOk);

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
  ASSERT_EQ(CoreAddAccount(session, &account, nullptr), kCoreOk);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreOk);

  CoreClearAccount(session);
}

TEST_F(CoreInterfaceTest, ChangeMasterPassword) {
  ASSERT_EQ(Open(), kCoreOk);
  const CoreAccount account = MakeAccount("d", "e", "u", "s3cr3t", "x");
  ASSERT_EQ(CoreAddAccount(session, &account, nullptr), kCoreOk);

  EXPECT_EQ(CoreChangeMasterPassword(session, "nope", 4, "new", 3, nullptr),
            kCoreWrongPassword);
  EXPECT_EQ(CoreChangeMasterPassword(session, kPassword, kPasswordSize, "new",
                                     0, nullptr),
            kCoreInvalidArgument);
  ASSERT_EQ(CoreChangeMasterPassword(session, kPassword, kPasswordSize, "new",
                                     3, &kTestKdf),
            kCoreOk);

  CoreClose(session);
  EXPECT_EQ(CoreUnlock(session, test_file.c_str(), kPassword, kPasswordSize),
            kCoreWrongPassword);
  ASSERT_EQ(CoreUnlock(session, test_file.c_str(), "new", 3), kCoreOk);
  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, IdAt(0), &fetched), kCoreOk);
  EXPECT_STREQ(fetched.password, "s3cr3t");
}

TEST_F(CoreInterfaceTest, ChangeMasterPasswordRejectsNullArguments) {
  ASSERT_EQ(Open(), kCoreOk);
  EXPECT_EQ(CoreChangeMasterPassword(nullptr, "a", 1, "b", 1, nullptr),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreChangeMasterPassword(session, nullptr, 1, "b", 1, nullptr),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreChangeMasterPassword(session, "a", 1, nullptr, 1, nullptr),
            kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, SearchCountAndCompact) {
  ASSERT_EQ(Open(), kCoreOk);
  const CoreAccount bank = MakeAccount("My Bank", "e", "u", "p", "x");
  const CoreAccount mail = MakeAccount("Email", "e", "u", "p", "x");
  const CoreAccount old_bank = MakeAccount("bank (old)", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(session, &bank, nullptr), kCoreOk);
  ASSERT_EQ(CoreAddAccount(session, &mail, nullptr), kCoreOk);
  ASSERT_EQ(CoreAddAccount(session, &old_bank, nullptr), kCoreOk);

  std::size_t count = 0;
  ASSERT_EQ(CoreCount(session, &count), kCoreOk);
  EXPECT_EQ(count, 3u);
  ASSERT_EQ(CoreLoadIndex(session, &count), kCoreOk);

  std::array<std::size_t, 1> indices{};
  ASSERT_EQ(CoreSearchIndex(session, "BANK", 4, indices.data(), indices.size(),
                            &count),
            kCoreOk);
  EXPECT_EQ(count, 2u);  // Total matches, even though only one fits.
  EXPECT_EQ(indices[0], 0u);

  ASSERT_EQ(CoreSearchIndex(session, nullptr, 0, nullptr, 0, &count), kCoreOk);
  EXPECT_EQ(count, 3u);

  EXPECT_EQ(CoreSearchIndex(session, nullptr, 1, nullptr, 0, &count),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreSearchIndex(session, "a", 1, nullptr, 1, &count),
            kCoreInvalidArgument);
  EXPECT_EQ(CoreSearchIndex(session, "a", 1, nullptr, 0, nullptr),
            kCoreInvalidArgument);

  EXPECT_EQ(CoreCompact(session), kCoreOk);
  EXPECT_EQ(CoreCompact(nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreCount(session, nullptr), kCoreInvalidArgument);
}

TEST_F(CoreInterfaceTest, AddAccountReportsTheIdUsedByTheIndex) {
  ASSERT_EQ(Open(), kCoreOk);
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  std::int64_t id = -1;
  ASSERT_EQ(CoreAddAccount(session, &account, &id), kCoreOk);

  std::size_t count = 0;
  ASSERT_EQ(CoreLoadIndex(session, &count), kCoreOk);
  CoreAccountEntry entry{};
  ASSERT_EQ(CoreGetIndexEntry(session, 0, &entry), kCoreOk);
  EXPECT_EQ(entry.id, id);
  EXPECT_EQ(entry.position, 0);

  CoreAccount fetched{};
  ASSERT_EQ(CoreGetAccount(session, id, &fetched), kCoreOk);
  ASSERT_EQ(CoreRemoveAccount(session, id), kCoreOk);
  EXPECT_EQ(CoreGetAccount(session, id, &fetched), kCoreNotFound);
}

TEST_F(CoreInterfaceTest, EditWithStaleRevisionReportsConflict) {
  ASSERT_EQ(Open(), kCoreOk);
  const CoreAccount original = MakeAccount("d", "e", "u", "p1", "x");
  std::int64_t id = -1;
  ASSERT_EQ(CoreAddAccount(session, &original, &id), kCoreOk);

  CoreAccount read{};
  ASSERT_EQ(CoreGetAccount(session, id, &read), kCoreOk);
  EXPECT_EQ(read.revision, 0);

  const CoreAccount first = MakeAccount("d", "e", "u", "p2", "x");
  ASSERT_EQ(CoreEditAccount(session, id, read.revision, &first), kCoreOk);
  const CoreAccount second = MakeAccount("d", "e", "u", "p3", "x");
  EXPECT_EQ(CoreEditAccount(session, id, read.revision, &second),
            kCoreConflict);

  ASSERT_EQ(CoreGetAccount(session, id, &read), kCoreOk);
  EXPECT_STREQ(read.password, "p2");
  EXPECT_EQ(read.revision, 1);

  std::size_t count = 0;
  ASSERT_EQ(CoreLoadIndex(session, &count), kCoreOk);
  CoreAccountEntry entry{};
  ASSERT_EQ(CoreGetIndexEntry(session, 0, &entry), kCoreOk);
  EXPECT_EQ(entry.revision, 1);
}

TEST_F(CoreInterfaceTest, CheckExternalChanges) {
  ASSERT_EQ(Open(), kCoreOk);
  bool changed = true;
  ASSERT_EQ(CoreCheckExternalChanges(session, &changed), kCoreOk);
  EXPECT_FALSE(changed);

  CoreSession* other = CoreNew();
  ASSERT_NE(other, nullptr);
  ASSERT_EQ(CoreUnlock(other, test_file.c_str(), kPassword, kPasswordSize),
            kCoreOk);
  const CoreAccount account = MakeAccount("d", "e", "u", "p", "x");
  ASSERT_EQ(CoreAddAccount(other, &account, nullptr), kCoreOk);
  CoreFree(other);

  ASSERT_EQ(CoreCheckExternalChanges(session, &changed), kCoreOk);
  EXPECT_TRUE(changed);

  EXPECT_EQ(CoreCheckExternalChanges(session, nullptr), kCoreInvalidArgument);
  EXPECT_EQ(CoreCheckExternalChanges(nullptr, &changed), kCoreInvalidArgument);
}
