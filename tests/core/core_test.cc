#include "core/core.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string_view>
#include <vector>

#include "db/db.h"

namespace {

constexpr std::string_view kPassword = "correct horse battery staple";
// Argon2id's minimum costs, so the tests do not spend seconds per vault.
constexpr Crypto::KdfParams kTestKdf{
    .memory_kib = 8, .iterations = 1, .parallelism = 1};

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

// Runs `sql` on `path` through a separate raw connection, simulating someone
// editing the vault file behind Core's back.
void Tamper(const std::string& path, const char* sql) {
  sqlite3* raw = nullptr;
  ASSERT_EQ(sqlite3_open(path.c_str(), &raw), SQLITE_OK);
  ASSERT_EQ(sqlite3_exec(raw, sql, nullptr, nullptr, nullptr), SQLITE_OK)
      << sqlite3_errmsg(raw);
  sqlite3_close(raw);
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

  void SetUp() override {
    std::remove(test_file.c_str());
  }

  void TearDown() override {
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
};

TEST_F(CoreTest, CreateMakesEmptyUnlockedVault) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  EXPECT_TRUE(core.IsOpen());
  EXPECT_TRUE(LoadedIndex(core).empty());
}

TEST_F(CoreTest, CreateFailsForUnreachablePath) {
  Core core;
  EXPECT_EQ(core.Create("/nonexistent_dir_xyz/unreachable.sqlite3", kPassword,
                        kTestKdf),
            CoreStatus::kError);
  EXPECT_FALSE(core.IsOpen());
}

TEST_F(CoreTest, OperationsReportLockedWhenVaultNotOpen) {
  Core core;
  ASSERT_FALSE(core.IsOpen());

  const AccountPlain account =
      MakeAccount("desc", "email", "user", "pass", "extra");

  EXPECT_EQ(core.AddAccount(account), CoreStatus::kLocked);
  EXPECT_EQ(core.EditAccount(IdAt(0), Core::kAnyRevision, account),
            CoreStatus::kLocked);
  EXPECT_EQ(core.RemoveAccount(IdAt(0)), CoreStatus::kLocked);
  EXPECT_EQ(core.ReorderAccount(IdAt(0), 1), CoreStatus::kLocked);
  EXPECT_EQ(core.LoadIndex(), CoreStatus::kLocked);
  EXPECT_TRUE(core.Index().empty());
  EXPECT_EQ(core.GetAccount(IdAt(0)).error(), CoreStatus::kLocked);
}

TEST_F(CoreTest, CloseWipesSessionState) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(
      core.AddAccount(MakeAccount("desc", "email", "user", "pass", "extra")),
      CoreStatus::kOk);
  ASSERT_NE(Fetch(core, IdAt(0)), nullptr);

  core.Close();

  EXPECT_FALSE(core.IsOpen());
  EXPECT_EQ(core.CurrentAccount(), nullptr);
  EXPECT_NE(
      core.AddAccount(MakeAccount("desc", "email", "user", "pass", "extra")),
      CoreStatus::kOk);
}

TEST_F(CoreTest, AddAndGetAccountRoundTrip) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);

  const AccountPlain original = MakeAccount("My Bank", "me@example.com", "me",
                                            "s3cr3t!", "some extra notes");
  ASSERT_EQ(core.AddAccount(original), CoreStatus::kOk);

  const AccountPlain* fetched = Fetch(core, IdAt(0));
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(GetAccountField(fetched->description), "My Bank");
  EXPECT_EQ(GetAccountField(fetched->email), "me@example.com");
  EXPECT_EQ(GetAccountField(fetched->username), "me");
  EXPECT_EQ(GetAccountField(fetched->password), "s3cr3t!");
  EXPECT_EQ(GetAccountField(fetched->extra), "some extra notes");
}

TEST_F(CoreTest, GetAccountReportsNotFoundForMissingPosition) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  EXPECT_EQ(core.GetAccount(IdAt(0)).error(), CoreStatus::kNotFound);
  EXPECT_EQ(core.GetAccount(IdAt(-1)).error(), CoreStatus::kNotFound);
  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreTest, OnlyOneAccountIsLoadedAtOnce) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("first", "e1", "u1", "p1", "x1")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("second", "e2", "u2", "p2", "x2")),
            CoreStatus::kOk);

  const AccountPlain* first = Fetch(core, IdAt(0));
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(GetAccountField(first->description), "first");

  // Loading a second account replaces the first; the returned pointer now
  // refers to the second account's data.
  const AccountPlain* second = Fetch(core, IdAt(1));
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(first, second);
  EXPECT_EQ(GetAccountField(second->description), "second");
  EXPECT_EQ(core.CurrentAccount(), second);
}

TEST_F(CoreTest, ClearAccountWipesLoadedAccount) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, IdAt(0)), nullptr);

  core.ClearAccount();

  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreTest, EditAccountOverwritesContent) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(
      core.AddAccount(MakeAccount("old", "old@e", "old_u", "old_p", "old_x")),
      CoreStatus::kOk);

  ASSERT_EQ(
      core.EditAccount(IdAt(0), Core::kAnyRevision,
                       MakeAccount("new", "new@e", "new_u", "new_p", "new_x")),
      CoreStatus::kOk);

  const AccountPlain* fetched = Fetch(core, IdAt(0));
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(GetAccountField(fetched->description), "new");
  EXPECT_EQ(GetAccountField(fetched->password), "new_p");
}

TEST_F(CoreTest, EditMissingAccountFails) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  EXPECT_EQ(core.EditAccount(IdAt(0), Core::kAnyRevision,
                             MakeAccount("desc", "e", "u", "p", "x")),
            CoreStatus::kNotFound);
}

TEST_F(CoreTest, EditingLoadedAccountClearsIt) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, IdAt(0)), nullptr);

  ASSERT_EQ(core.EditAccount(IdAt(0), Core::kAnyRevision,
                             MakeAccount("new", "e", "u", "p", "x")),
            CoreStatus::kOk);

  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreTest, EditingOtherAccountKeepsLoadedAccountIntact) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("first", "e1", "u1", "p1", "x1")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("second", "e2", "u2", "p2", "x2")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, IdAt(0)), nullptr);

  ASSERT_EQ(core.EditAccount(IdAt(1), Core::kAnyRevision,
                             MakeAccount("edited", "e", "u", "p", "x")),
            CoreStatus::kOk);

  ASSERT_NE(core.CurrentAccount(), nullptr);
  EXPECT_EQ(GetAccountField(core.CurrentAccount()->description), "first");
}

TEST_F(CoreTest, RemoveAccountShiftsPositionsAndClearsCaches) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("first", "e1", "u1", "p1", "x1")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("second", "e2", "u2", "p2", "x2")),
            CoreStatus::kOk);
  ASSERT_NE(Fetch(core, IdAt(0)), nullptr);

  ASSERT_EQ(core.RemoveAccount(IdAt(0)), CoreStatus::kOk);

  EXPECT_EQ(core.CurrentAccount(), nullptr);
  const AccountPlain* fetched = Fetch(core, IdAt(0));
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(GetAccountField(fetched->description), "second");
}

TEST_F(CoreTest, RemoveMissingAccountFails) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  EXPECT_EQ(core.RemoveAccount(IdAt(0)), CoreStatus::kNotFound);
}

TEST_F(CoreTest, ReorderAccountMovesEntries) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("b", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("c", "e", "u", "p", "x")),
            CoreStatus::kOk);

  ASSERT_EQ(core.ReorderAccount(IdAt(0), 2), CoreStatus::kOk);

  const std::vector<AccountEntry>& index = LoadedIndex(core);
  ASSERT_EQ(index.size(), 3u);
  EXPECT_EQ(GetAccountField(index[0].description), "b");
  EXPECT_EQ(GetAccountField(index[1].description), "c");
  EXPECT_EQ(GetAccountField(index[2].description), "a");
}

TEST_F(CoreTest, ReorderOutOfRangeFails) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  EXPECT_EQ(core.ReorderAccount(IdAt(0), 5), CoreStatus::kNotFound);
}

TEST_F(CoreTest, GetAllIndexReturnsOrderedDecryptedDescriptions) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
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
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
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
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("alpha", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(LoadedIndex(core).size(), 1u);

  ASSERT_EQ(core.AddAccount(MakeAccount("beta", "e", "u", "p", "x")),
            CoreStatus::kOk);

  EXPECT_TRUE(core.Index().empty());
  EXPECT_EQ(LoadedIndex(core).size(), 2u);
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
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  const AccountPlain account = MakeAccount("desc", "e", "u", "p", "x");
  ASSERT_EQ(core.AddAccount(account), CoreStatus::kOk);

  DB db;
  ASSERT_TRUE(db.Open(test_file));
  const auto before = db.GetAccount(IdAt(0));
  ASSERT_TRUE(before.has_value());

  // Re-encrypting identical plaintext must not reuse the (key, IV) pair.
  ASSERT_EQ(core.EditAccount(IdAt(0), Core::kAnyRevision, account),
            CoreStatus::kOk);
  const auto after = db.GetAccount(IdAt(0));
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(before->uid, after->uid);
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

TEST_F(CoreTest, CreateRejectsEmptyPassword) {
  Core core;
  EXPECT_EQ(core.Create(test_file, "", kTestKdf), CoreStatus::kInvalidArgument);
  EXPECT_FALSE(core.IsOpen());
}

TEST_F(CoreTest, CreateRejectsInvalidKdfParams) {
  Core core;
  EXPECT_EQ(core.Create(test_file, kPassword,
                        {.memory_kib = 1, .iterations = 1, .parallelism = 1}),
            CoreStatus::kInvalidArgument);
  EXPECT_FALSE(core.IsOpen());
}

TEST_F(CoreTest, CreateFailsIfVaultAlreadyExists) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
              CoreStatus::kOk);
  }

  Core core;
  EXPECT_EQ(core.Create(test_file, "another password", kTestKdf),
            CoreStatus::kAlreadyExists);
  EXPECT_FALSE(core.IsOpen());

  // The original vault is untouched.
  ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  EXPECT_EQ(core.Index().size(), 1u);
}

TEST_F(CoreTest, CreateRefusesFileWithAccountsButNoMeta) {
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    ASSERT_EQ(db.AddAccount(AccountBinary{}), DBStatus::kOk);
  }
  Core core;
  EXPECT_EQ(core.Create(test_file, kPassword, kTestKdf),
            CoreStatus::kAlreadyExists);
}

TEST_F(CoreTest, CreateStoresKdfParamsAndRandomSalt) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  }
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  const auto meta = db.GetMeta();
  ASSERT_TRUE(meta.has_value());
  EXPECT_EQ(meta->version, VaultMeta::kCurrentVersion);
  EXPECT_EQ(meta->kdf_params, kTestKdf);
  EXPECT_NE(meta->kdf_salt, decltype(meta->kdf_salt){});
  EXPECT_EQ(meta->key_epoch, 0);
}

TEST_F(CoreTest, UnlockWithCorrectPasswordReadsAccounts) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "secret", "x")),
              CoreStatus::kOk);
  }

  Core core;
  ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
  EXPECT_TRUE(core.IsOpen());
  const AccountPlain* account = Fetch(core, IdAt(0));
  ASSERT_NE(account, nullptr);
  EXPECT_EQ(GetAccountField(account->password), "secret");
}

TEST_F(CoreTest, UnlockWithWrongPasswordFailsAndStaysLocked) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("desc", "e", "u", "p", "x")),
              CoreStatus::kOk);
  }

  Core core;
  EXPECT_EQ(core.Unlock(test_file, "wrong password"),
            CoreStatus::kWrongPassword);
  EXPECT_FALSE(core.IsOpen());
  // Nothing can be written with the wrong key.
  EXPECT_EQ(core.AddAccount(MakeAccount("d", "e", "u", "p", "x")),
            CoreStatus::kLocked);
  EXPECT_EQ(core.Unlock(test_file, ""), CoreStatus::kWrongPassword);
}

TEST_F(CoreTest, UnlockMissingVaultReportsNotFoundWithoutCreatingIt) {
  Core core;
  EXPECT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kNotFound);
  EXPECT_FALSE(std::filesystem::exists(test_file));
}

TEST_F(CoreTest, UnlockDatabaseWithoutMetaReportsNotFound) {
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
  }
  Core core;
  EXPECT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kNotFound);
}

TEST_F(CoreTest, UnlockReportsCorruptedForUnknownVersion) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  }
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    VaultMeta meta = db.GetMeta().value();
    meta.version = VaultMeta::kCurrentVersion + 1;
    ASSERT_EQ(db.UpdateMeta(meta), DBStatus::kOk);
  }
  Core core;
  EXPECT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kCorrupted);
}

TEST_F(CoreTest, TamperedVerifierIsRejected) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  }
  {
    DB db;
    ASSERT_TRUE(db.Open(test_file));
    VaultMeta meta = db.GetMeta().value();
    meta.verifier[0] ^= std::byte{0x01};
    ASSERT_EQ(db.UpdateMeta(meta), DBStatus::kOk);
  }
  Core core;
  EXPECT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kWrongPassword);
}

TEST_F(CoreTest, DefaultKdfParamsAreUsedWhenNotGiven) {
  {
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword), CoreStatus::kOk);
  }
  DB db;
  ASSERT_TRUE(db.Open(test_file));
  EXPECT_EQ(db.GetMeta()->kdf_params, Crypto::kDefaultKdfParams);
}

TEST_F(CoreTest, EachAccountGetsARandomUid) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);

  DB db;
  ASSERT_TRUE(db.Open(test_file));
  const auto first = db.GetAccount(IdAt(0));
  const auto second = db.GetAccount(IdAt(1));
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_NE(first->uid, second->uid);
  EXPECT_NE(first->uid, decltype(first->uid){});
}

TEST_F(CoreTest, CiphertextsMovedBetweenAccountsFailAuthentication) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("bank", "e", "u", "bank-pw", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("mail", "e", "u", "mail-pw", "x")),
            CoreStatus::kOk);

  // Give the "mail" row the ciphertexts of the "bank" row (keeping its own
  // uid): every field is validly encrypted under the vault key, but for
  // another account.
  Tamper(test_file,
         "UPDATE accounts SET "
         "  description = (SELECT description FROM accounts WHERE position = "
         "0),"
         "  data = (SELECT data FROM accounts WHERE position = 0),"
         "  iv_description = (SELECT iv_description FROM accounts WHERE "
         "position = 0),"
         "  iv_data = (SELECT iv_data FROM accounts WHERE position = 0),"
         "  tag_description = (SELECT tag_description FROM accounts WHERE "
         "position = 0),"
         "  tag_data = (SELECT tag_data FROM accounts WHERE position = 0) "
         "WHERE position = 1;");

  EXPECT_EQ(core.GetAccount(IdAt(1)).error(), CoreStatus::kCorrupted);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  ASSERT_EQ(core.Index().size(), 1u);
  EXPECT_EQ(GetAccountField(core.Index()[0].description), "bank");
}

TEST_F(CoreTest, DescriptionAndDataCannotBeSwapped) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("b", "e", "u", "p", "x")),
            CoreStatus::kOk);

  // Swap only the description ciphertext (with its IV and tag) of the two
  // accounts.
  Tamper(test_file,
         "CREATE TEMP TABLE d AS SELECT position, description, iv_description, "
         "tag_description FROM accounts;"
         "UPDATE accounts SET "
         "  description = (SELECT description FROM d WHERE d.position = 1 - "
         "accounts.position),"
         "  iv_description = (SELECT iv_description FROM d WHERE d.position = "
         "1 - accounts.position),"
         "  tag_description = (SELECT tag_description FROM d WHERE "
         "d.position = 1 - accounts.position);");

  EXPECT_EQ(core.GetAccount(IdAt(0)).error(), CoreStatus::kCorrupted);
  EXPECT_EQ(core.GetAccount(IdAt(1)).error(), CoreStatus::kCorrupted);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  EXPECT_TRUE(core.Index().empty());
}

class CoreChangePasswordTest : public CoreTest {
 protected:
  static constexpr std::string_view kNewPassword = "a brand new password";

  // Creates the vault with two accounts and leaves `core` unlocked.
  void Populate(Core& core) const {
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("bank", "e1", "u1", "pw1", "x1")),
              CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("mail", "e2", "u2", "pw2", "x2")),
              CoreStatus::kOk);
  }

  VaultMeta ReadMeta() const {
    DB db;
    EXPECT_TRUE(db.Open(test_file));
    return db.GetMeta().value();
  }
};

TEST_F(CoreChangePasswordTest, NewPasswordOpensEverythingAndOldOneFails) {
  {
    Core core;
    Populate(core);
    ASSERT_EQ(core.ChangeMasterPassword(kPassword, kNewPassword),
              CoreStatus::kOk);
    // The session keeps working with the new key.
    ASSERT_NE(Fetch(core, IdAt(1)), nullptr);
    EXPECT_EQ(GetAccountField(Fetch(core, IdAt(1))->password), "pw2");
    ASSERT_EQ(core.AddAccount(MakeAccount("new", "e", "u", "p3", "x")),
              CoreStatus::kOk);
  }

  Core core;
  EXPECT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kWrongPassword);
  ASSERT_EQ(core.Unlock(test_file, kNewPassword), CoreStatus::kOk);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  ASSERT_EQ(core.Index().size(), 3u);
  EXPECT_EQ(GetAccountField(Fetch(core, IdAt(0))->password), "pw1");
  EXPECT_EQ(GetAccountField(Fetch(core, IdAt(1))->password), "pw2");
  EXPECT_EQ(GetAccountField(Fetch(core, IdAt(2))->password), "p3");
}

TEST_F(CoreChangePasswordTest, BumpsKeyEpochAndRenewsSalt) {
  Core core;
  Populate(core);
  const VaultMeta before = ReadMeta();

  ASSERT_EQ(core.ChangeMasterPassword(kPassword, kNewPassword),
            CoreStatus::kOk);

  const VaultMeta after = ReadMeta();
  EXPECT_EQ(after.key_epoch, before.key_epoch + 1);
  EXPECT_NE(after.kdf_salt, before.kdf_salt);
  EXPECT_NE(after.verifier_tag, before.verifier_tag);
  EXPECT_EQ(after.kdf_params, before.kdf_params);
}

TEST_F(CoreChangePasswordTest, CanChangeKdfParams) {
  Core core;
  Populate(core);
  constexpr Crypto::KdfParams kStronger{
      .memory_kib = 16, .iterations = 2, .parallelism = 1};

  ASSERT_EQ(core.ChangeMasterPassword(kPassword, kNewPassword, kStronger),
            CoreStatus::kOk);
  EXPECT_EQ(ReadMeta().kdf_params, kStronger);
  core.Close();
  EXPECT_EQ(core.Unlock(test_file, kNewPassword), CoreStatus::kOk);
}

TEST_F(CoreChangePasswordTest, WrongCurrentPasswordChangesNothing) {
  Core core;
  Populate(core);
  const VaultMeta before = ReadMeta();

  EXPECT_EQ(core.ChangeMasterPassword("not it", kNewPassword),
            CoreStatus::kWrongPassword);
  EXPECT_EQ(ReadMeta(), before);
  EXPECT_NE(Fetch(core, IdAt(0)), nullptr);
}

TEST_F(CoreChangePasswordTest, RejectsEmptyNewPasswordAndBadParams) {
  Core core;
  Populate(core);
  const VaultMeta before = ReadMeta();

  EXPECT_EQ(core.ChangeMasterPassword(kPassword, ""),
            CoreStatus::kInvalidArgument);
  EXPECT_EQ(core.ChangeMasterPassword(
                kPassword, kNewPassword,
                Crypto::KdfParams{
                    .memory_kib = 1, .iterations = 1, .parallelism = 1}),
            CoreStatus::kInvalidArgument);
  EXPECT_EQ(ReadMeta(), before);
}

TEST_F(CoreChangePasswordTest, FailureHalfwayLeavesTheVaultUntouched) {
  Core core;
  Populate(core);
  const VaultMeta before = ReadMeta();

  // Corrupt the second account: the first one gets re-encrypted before the
  // second fails to decrypt, so the transaction must roll that back.
  Tamper(test_file,
         "UPDATE accounts SET tag_data = zeroblob(16) WHERE position = 1;");

  EXPECT_EQ(core.ChangeMasterPassword(kPassword, kNewPassword),
            CoreStatus::kCorrupted);
  EXPECT_EQ(ReadMeta(), before);

  // The session still uses the old key, and so does the file.
  EXPECT_EQ(GetAccountField(Fetch(core, IdAt(0))->password), "pw1");
  core.Close();
  EXPECT_EQ(core.Unlock(test_file, kNewPassword), CoreStatus::kWrongPassword);
  ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
  EXPECT_EQ(GetAccountField(Fetch(core, IdAt(0))->password), "pw1");
}

TEST_F(CoreChangePasswordTest, ClearsCachedPlaintext) {
  Core core;
  Populate(core);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  ASSERT_NE(Fetch(core, IdAt(0)), nullptr);

  ASSERT_EQ(core.ChangeMasterPassword(kPassword, kNewPassword),
            CoreStatus::kOk);
  EXPECT_TRUE(core.Index().empty());
  EXPECT_EQ(core.CurrentAccount(), nullptr);
}

TEST_F(CoreChangePasswordTest, RequiresAnOpenVault) {
  Core core;
  EXPECT_EQ(core.ChangeMasterPassword(kPassword, kNewPassword),
            CoreStatus::kLocked);
}

TEST_F(CoreTest, SearchIndexMatchesSubstringsIgnoringCase) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  for (const char* description : {"My Bank", "Email", "bank (old)", "Gym"}) {
    ASSERT_EQ(core.AddAccount(MakeAccount(description, "e", "u", "p", "x")),
              CoreStatus::kOk);
  }
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);

  EXPECT_EQ(core.SearchIndex("BANK"), (std::vector<std::size_t>{0, 2}));
  EXPECT_EQ(core.SearchIndex("aIl"), (std::vector<std::size_t>{1}));
  EXPECT_EQ(core.SearchIndex(""), (std::vector<std::size_t>{0, 1, 2, 3}));
  EXPECT_TRUE(core.SearchIndex("nothing").empty());

  core.ClearIndex();
  EXPECT_TRUE(core.SearchIndex("").empty());
}

TEST_F(CoreTest, CountAndCompact) {
  Core core;
  EXPECT_EQ(core.Count().error(), CoreStatus::kLocked);
  EXPECT_EQ(core.Compact(), CoreStatus::kLocked);

  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  EXPECT_EQ(core.Count().value(), 0u);
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("b", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.RemoveAccount(IdAt(0)), CoreStatus::kOk);
  EXPECT_EQ(core.Count().value(), 1u);

  EXPECT_EQ(core.Compact(), CoreStatus::kOk);
  EXPECT_EQ(core.Count().value(), 1u);
  EXPECT_EQ(GetAccountField(Fetch(core, IdAt(0))->description), "b");
}

TEST_F(CoreTest, AddAccountReportsTheIdUsedByTheIndex) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  std::int64_t first = -1;
  std::int64_t second = -1;
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x"), &first),
            CoreStatus::kOk);
  ASSERT_EQ(core.AddAccount(MakeAccount("b", "e", "u", "p", "x"), &second),
            CoreStatus::kOk);

  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  ASSERT_EQ(core.Index().size(), 2u);
  EXPECT_EQ(core.Index()[0].id, first);
  EXPECT_EQ(core.Index()[0].position, 0);
  EXPECT_EQ(core.Index()[1].id, second);
  EXPECT_EQ(core.Index()[1].position, 1);
}

TEST_F(CoreTest, IdsKeepNamingTheSameAccountWhenAnotherSessionChangesTheList) {
  Core gui;
  ASSERT_EQ(gui.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  for (const char* name : {"first", "second", "third", "fourth"}) {
    ASSERT_EQ(gui.AddAccount(MakeAccount(name, "e", "u", "p", "x")),
              CoreStatus::kOk);
  }
  ASSERT_EQ(gui.LoadIndex(), CoreStatus::kOk);
  const std::int64_t fourth = gui.Index()[3].id;
  const std::int64_t first = gui.Index()[0].id;

  // Meanwhile another program removes the first account and moves the last
  // one to the top: every position the GUI is showing is now stale.
  Core cli;
  ASSERT_EQ(cli.Unlock(test_file, kPassword), CoreStatus::kOk);
  ASSERT_EQ(cli.RemoveAccount(first), CoreStatus::kOk);
  ASSERT_EQ(cli.ReorderAccount(fourth, 0), CoreStatus::kOk);

  // Editing "the fourth account" still edits the fourth account...
  ASSERT_EQ(gui.EditAccount(fourth, Core::kAnyRevision,
                            MakeAccount("fourth*", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(gui.LoadIndex(), CoreStatus::kOk);
  ASSERT_EQ(gui.Index().size(), 3u);
  EXPECT_EQ(GetAccountField(gui.Index()[0].description), "fourth*");
  EXPECT_EQ(GetAccountField(gui.Index()[1].description), "second");
  EXPECT_EQ(GetAccountField(gui.Index()[2].description), "third");

  // ...and acting on the removed one fails instead of hitting another one.
  EXPECT_EQ(gui.EditAccount(first, Core::kAnyRevision,
                            MakeAccount("x", "e", "u", "p", "x")),
            CoreStatus::kNotFound);
  EXPECT_EQ(gui.RemoveAccount(first), CoreStatus::kNotFound);
  EXPECT_EQ(gui.GetAccount(first).error(), CoreStatus::kNotFound);
}

TEST_F(CoreTest, ConcurrentEditsOfTheSameAccountAreDetected) {
  Core gui;
  ASSERT_EQ(gui.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  std::int64_t id = -1;
  ASSERT_EQ(gui.AddAccount(MakeAccount("bank", "e", "u", "old-pw", "x"), &id),
            CoreStatus::kOk);
  Core cli;
  ASSERT_EQ(cli.Unlock(test_file, kPassword), CoreStatus::kOk);

  // Both programs open the account at the same revision.
  const auto gui_view = gui.GetAccount(id);
  const auto cli_view = cli.GetAccount(id);
  ASSERT_TRUE(gui_view.has_value());
  ASSERT_TRUE(cli_view.has_value());
  const std::int64_t read_revision = (*gui_view)->revision;
  EXPECT_EQ((*cli_view)->revision, read_revision);

  // The CLI saves first.
  ASSERT_EQ(cli.EditAccount(id, read_revision,
                            MakeAccount("bank", "e", "u", "cli-pw", "x")),
            CoreStatus::kOk);

  // The GUI's save would overwrite the CLI's change: it is refused.
  EXPECT_EQ(gui.EditAccount(id, read_revision,
                            MakeAccount("bank", "e", "u", "gui-pw", "x")),
            CoreStatus::kConflict);
  const AccountPlain* stored = Fetch(gui, id);
  ASSERT_NE(stored, nullptr);
  EXPECT_EQ(GetAccountField(stored->password), "cli-pw");
  EXPECT_EQ(stored->revision, read_revision + 1);

  // After reloading, the GUI can save on top of the newer version.
  EXPECT_EQ(gui.EditAccount(id, stored->revision,
                            MakeAccount("bank", "e", "u", "gui-pw", "x")),
            CoreStatus::kOk);
  EXPECT_EQ(GetAccountField(Fetch(cli, id)->password), "gui-pw");
}

TEST_F(CoreTest, IndexReportsRevisions) {
  Core core;
  ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  std::int64_t id = -1;
  ASSERT_EQ(core.AddAccount(MakeAccount("a", "e", "u", "p", "x"), &id),
            CoreStatus::kOk);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  EXPECT_EQ(core.Index()[0].revision, 0);

  ASSERT_EQ(core.EditAccount(id, 0, MakeAccount("b", "e", "u", "p", "x")),
            CoreStatus::kOk);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  EXPECT_EQ(core.Index()[0].revision, 1);
}
