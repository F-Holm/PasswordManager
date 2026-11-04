#include "core/core.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace {

std::array<std::byte, Crypto::Size::kKey> MakeKey(std::byte marker) {
  std::array<std::byte, Crypto::Size::kKey> key{};
  key.fill(marker);
  return key;
}

AccountPlain MakeAccount(std::string_view description, std::string_view email,
                         std::string_view username, std::string_view password,
                         std::string_view extra) {
  AccountPlain account;
  EXPECT_TRUE(SetAccountField(account.description, description));
  EXPECT_TRUE(SetAccountField(account.email, email));
  EXPECT_TRUE(SetAccountField(account.username, username));
  EXPECT_TRUE(SetAccountField(account.password, password));
  EXPECT_TRUE(SetAccountField(account.extra, extra));
  return account;
}

// Returns the account at `position`, or nullptr if GetAccount fails.
const AccountPlain* Fetch(Core& core, std::int64_t position) {
  const auto account = core.GetAccount(position);
  return account.has_value() ? *account : nullptr;
}

// Loads the index (expecting success) and returns it.
const std::vector<AccountEntry>& LoadedIndex(Core& core) {
  EXPECT_EQ(core.LoadIndex(), CoreStatus::kOk);
  return core.Index();
}

}  // namespace

class CoreTest : public ::testing::Test {
 protected:
  // Each ctest case runs as a separate process of the same binary sharing
  // the working directory, so the file name must be unique per test to stay
  // parallel-safe (see `just test`).
  const std::string test_file =
      std::string("core_test_") +
      ::testing::UnitTest::GetInstance()->current_test_info()->name() +
      ".sqlite3";
  const std::array<std::byte, Crypto::Size::kKey> key =
      MakeKey(std::byte{0x42});

  void SetUp() override {
    std::remove(test_file.c_str());
  }

  void TearDown() override {
    std::remove(test_file.c_str());
  }
};

TEST_F(CoreTest, OpenCreatesEmptyVault) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  EXPECT_TRUE(core.IsOpen());
  EXPECT_TRUE(LoadedIndex(core).empty());
}

TEST_F(CoreTest, OpenFailsForUnreachablePath) {
  Core core;
  EXPECT_EQ(core.Open("/nonexistent_dir_xyz/unreachable.sqlite3", key),
            CoreStatus::kError);
  EXPECT_FALSE(core.IsOpen());
}

TEST_F(CoreTest, OperationsReportLockedWhenVaultNotOpen) {
  Core core;
  ASSERT_FALSE(core.IsOpen());

  const AccountPlain account =
      MakeAccount("desc", "email", "user", "pass", "extra");

  EXPECT_EQ(core.AddAccount(account), CoreStatus::kLocked);
  EXPECT_EQ(core.EditAccount(0, account), CoreStatus::kLocked);
  EXPECT_EQ(core.RemoveAccount(0), CoreStatus::kLocked);
  EXPECT_EQ(core.ReorderAccount(0, 1), CoreStatus::kLocked);
  EXPECT_EQ(core.LoadIndex(), CoreStatus::kLocked);
  EXPECT_TRUE(core.Index().empty());
  EXPECT_EQ(core.GetAccount(0).error(), CoreStatus::kLocked);
}

TEST_F(CoreTest, CloseWipesSessionState) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(
      core.AddAccount(MakeAccount("desc", "email", "user", "pass", "extra")),
      CoreStatus::kOk);
  ASSERT_NE(Fetch(core, 0), nullptr);

  core.Close();

  EXPECT_FALSE(core.IsOpen());
  EXPECT_EQ(core.CurrentAccount(), nullptr);
  EXPECT_NE(
      core.AddAccount(MakeAccount("desc", "email", "user", "pass", "extra")),
      CoreStatus::kOk);
}

TEST_F(CoreTest, AddAndGetAccountRoundTrip) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);

  const AccountPlain original = MakeAccount("My Bank", "me@example.com", "me",
                                            "s3cr3t!", "some extra notes");
  ASSERT_EQ(core.AddAccount(original), CoreStatus::kOk);

  const AccountPlain* fetched = Fetch(core, 0);
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(GetAccountField(fetched->description), "My Bank");
  EXPECT_EQ(GetAccountField(fetched->email), "me@example.com");
  EXPECT_EQ(GetAccountField(fetched->username), "me");
  EXPECT_EQ(GetAccountField(fetched->password), "s3cr3t!");
  EXPECT_EQ(GetAccountField(fetched->extra), "some extra notes");
}

TEST_F(CoreTest, GetAccountReportsNotFoundForMissingPosition) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  EXPECT_EQ(core.GetAccount(0).error(), CoreStatus::kNotFound);
  EXPECT_EQ(core.GetAccount(-1).error(), CoreStatus::kNotFound);
  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreTest, OnlyOneAccountIsLoadedAtOnce) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("first", "e1", "u1", "p1", "x1")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("second", "e2", "u2", "p2", "x2")),
            CoreStatus::kOk);

  const AccountPlain* first = Fetch(core, 0);
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(GetAccountField(first->description), "first");

  // Loading a second account replaces the first; the returned pointer now
  // refers to the second account's data.
  const AccountPlain* second = Fetch(core, 1);
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(first, second);
  EXPECT_EQ(GetAccountField(second->description), "second");
  EXPECT_EQ(core.CurrentAccount(), second);
}

TEST_F(CoreTest, ClearAccountWipesLoadedAccount) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, 0), nullptr);

  core.ClearAccount();

  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreTest, EditAccountOverwritesContent) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(
      core.AddAccount(MakeAccount("old", "old@e", "old_u", "old_p", "old_x")),
      CoreStatus::kOk);

  ASSERT_EQ(core.EditAccount(
                0, MakeAccount("new", "new@e", "new_u", "new_p", "new_x")),
            CoreStatus::kOk);

  const AccountPlain* fetched = Fetch(core, 0);
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(GetAccountField(fetched->description), "new");
  EXPECT_EQ(GetAccountField(fetched->password), "new_p");
}

TEST_F(CoreTest, EditMissingAccountFails) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  EXPECT_EQ(core.EditAccount(0, MakeAccount("desc", "e", "u", "p", "x")),
            CoreStatus::kNotFound);
}

TEST_F(CoreTest, EditingLoadedAccountClearsIt) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, 0), nullptr);

  ASSERT_EQ(core.EditAccount(0, MakeAccount("new", "e", "u", "p", "x")),
            CoreStatus::kOk);

  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreTest, EditingOtherAccountKeepsLoadedAccountIntact) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("first", "e1", "u1", "p1", "x1")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("second", "e2", "u2", "p2", "x2")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, 0), nullptr);

  ASSERT_EQ(core.EditAccount(1, MakeAccount("edited", "e", "u", "p", "x")),
            CoreStatus::kOk);

  ASSERT_NE(core.CurrentAccount(), nullptr);
  EXPECT_EQ(GetAccountField(core.CurrentAccount()->description), "first");
}

TEST_F(CoreTest, RemoveAccountShiftsPositionsAndClearsCaches) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("first", "e1", "u1", "p1", "x1")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("second", "e2", "u2", "p2", "x2")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, 0), nullptr);

  ASSERT_EQ(core.RemoveAccount(0), CoreStatus::kOk);

  EXPECT_EQ(core.CurrentAccount(), nullptr);
  const AccountPlain* fetched = Fetch(core, 0);
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(GetAccountField(fetched->description), "second");
}

TEST_F(CoreTest, RemoveMissingAccountFails) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  EXPECT_EQ(core.RemoveAccount(0), CoreStatus::kNotFound);
}

TEST_F(CoreTest, ReorderAccountMovesEntries) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("b", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("c", "e", "u", "p", "x")),
            CoreStatus::kOk);

  ASSERT_EQ(core.ReorderAccount(0, 2), CoreStatus::kOk);

  const std::vector<AccountEntry>& index = LoadedIndex(core);
  ASSERT_EQ(index.size(), 3u);
  EXPECT_EQ(GetAccountField(index[0].description), "b");
  EXPECT_EQ(GetAccountField(index[1].description), "c");
  EXPECT_EQ(GetAccountField(index[2].description), "a");
}

TEST_F(CoreTest, ReorderOutOfRangeFails) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  EXPECT_EQ(core.ReorderAccount(0, 5), CoreStatus::kNotFound);
}

TEST_F(CoreTest, GetAllIndexReturnsOrderedDecryptedDescriptions) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("alpha", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("beta", "e", "u", "p", "x")),
            CoreStatus::kOk);

  const std::vector<AccountEntry>& index = LoadedIndex(core);

  ASSERT_EQ(index.size(), 2u);
  EXPECT_EQ(index[0].position, 0);
  EXPECT_EQ(GetAccountField(index[0].description), "alpha");
  EXPECT_EQ(index[1].position, 1);
  EXPECT_EQ(GetAccountField(index[1].description), "beta");
}

TEST_F(CoreTest, IndexReflectsLastLoadIndexCall) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("alpha", "e", "u", "p", "x")),
            CoreStatus::kOk);

  EXPECT_TRUE(core.Index().empty());
  LoadedIndex(core);
  EXPECT_EQ(core.Index().size(), 1u);

  core.ClearIndex();
  EXPECT_TRUE(core.Index().empty());
}

TEST_F(CoreTest, AddAccountInvalidatesIndexCache) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("alpha", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(LoadedIndex(core).size(), 1u);

  ASSERT_EQ(core.AddAccount(MakeAccount("beta", "e", "u", "p", "x")),
            CoreStatus::kOk);

  EXPECT_TRUE(core.Index().empty());
  EXPECT_EQ(LoadedIndex(core).size(), 2u);
}

TEST_F(CoreTest, WrongMasterKeyFailsToDecryptAccount) {
  {
    Core core;
    ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
              CoreStatus::kOk);
  }

  Core core;
  ASSERT_EQ(core.Open(test_file, MakeKey(std::byte{0x99})), CoreStatus::kOk);
  EXPECT_EQ(core.GetAccount(0).error(), CoreStatus::kCorrupted);
}

TEST_F(CoreTest, WrongMasterKeyOmitsUndecryptableIndexEntries) {
  {
    Core core;
    ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
              CoreStatus::kOk);
  }

  Core core;
  ASSERT_EQ(core.Open(test_file, MakeKey(std::byte{0x99})), CoreStatus::kOk);
  EXPECT_TRUE(LoadedIndex(core).empty());
}

TEST_F(CoreTest, SetAccountFieldRejectsOversizedValue) {
  AccountPlain account;
  const std::string too_long(account.username.size() + 1, 'a');
  EXPECT_FALSE(SetAccountField(account.username, too_long));
}

TEST_F(CoreTest, SetAccountFieldZeroPadsRemainder) {
  AccountPlain account;
  ASSERT_TRUE(SetAccountField(account.username, "ab"));
  EXPECT_EQ(account.username[2], '\0');
  EXPECT_EQ(account.username[account.username.size() - 1], '\0');
}

TEST_F(CoreTest, GetAccountFieldTrimsAtFirstNul) {
  AccountPlain account;
  ASSERT_TRUE(SetAccountField(account.username, "bob"));
  EXPECT_EQ(GetAccountField(account.username), "bob");
}

TEST_F(CoreTest, AccountPlainDestructorClearsPassword) {
  alignas(AccountPlain) char buffer[sizeof(AccountPlain)];

  {
    AccountPlain* account = new (buffer) AccountPlain();
    ASSERT_TRUE(SetAccountField(account->password, "s3cr3t"));
    account->~AccountPlain();
  }

  const auto* raw = reinterpret_cast<const char*>(buffer);
  const bool all_zero = std::all_of(raw, raw + sizeof(AccountPlain),
                                    [](char c) { return c == '\0'; });
  EXPECT_TRUE(all_zero) << "Destructor did not wipe AccountPlain";
}

TEST_F(CoreTest, EditingDrawsFreshIvs) {
  Core core;
  ASSERT_EQ(core.Open(test_file, key), CoreStatus::kOk);
  const AccountPlain account = MakeAccount("desc", "e", "u", "p", "x");
  ASSERT_EQ(core.AddAccount(account), CoreStatus::kOk);

  DB db;
  ASSERT_TRUE(db.Open(test_file));
  const auto before = db.GetAccount(0);
  ASSERT_TRUE(before.has_value());

  // Re-encrypting identical plaintext must not reuse the (key, IV) pair.
  ASSERT_EQ(core.EditAccount(0, account), CoreStatus::kOk);
  const auto after = db.GetAccount(0);
  ASSERT_TRUE(after.has_value());
  EXPECT_NE(before->iv_description, after->iv_description);
  EXPECT_NE(before->iv_data, after->iv_data);
  EXPECT_NE(before->data, after->data);
}

TEST(CoreStatusTest, EveryStatusHasAName) {
  EXPECT_EQ(ToString(CoreStatus::kOk), "ok");
  EXPECT_EQ(ToString(CoreStatus::kBusy), "busy");
  EXPECT_EQ(ToString(CoreStatus::kWrongPassword), "wrong password");
  EXPECT_EQ(ToString(CoreStatus::kError), "error");
}
