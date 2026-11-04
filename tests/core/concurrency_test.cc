// Several sessions (one per thread, as separate programs would have) using
// the same vault file at the same time. Also run under ThreadSanitizer by
// `just sanitize tsan`.
#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <latch>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "core/core.h"
#include "db/db.h"

namespace {

constexpr std::string_view kPassword = "shared vault password";
constexpr Crypto::KdfParams kTestKdf{
    .memory_kib = 8, .iterations = 1, .parallelism = 1};
constexpr int kThreads = 4;

AccountPlain MakeAccount(std::string_view description,
                         std::string_view password) {
  AccountPlain account;
  EXPECT_TRUE(SetAccountField(account.description, description));
  EXPECT_TRUE(SetAccountField(account.password, password));
  return account;
}

class ConcurrencyTest : public ::testing::Test {
 protected:
  const std::string test_file =
      std::string("concurrency_test_") +
      ::testing::UnitTest::GetInstance()->current_test_info()->name() +
      ".sqlite3";

  void SetUp() override {
    std::remove(test_file.c_str());
    Core core;
    ASSERT_EQ(core.Create(test_file, kPassword, kTestKdf), CoreStatus::kOk);
  }

  void TearDown() override {
    std::remove(test_file.c_str());
    std::remove((test_file + "-wal").c_str());
    std::remove((test_file + "-shm").c_str());
  }

  // Runs `body(thread_index)` on kThreads threads at once.
  template <typename Body>
  static void RunInParallel(Body body) {
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
      threads.emplace_back(body, i);
    }
    for (std::thread& thread : threads) {
      thread.join();
    }
  }
};

}  // namespace

TEST_F(ConcurrencyTest, ParallelAddsKeepEveryAccountAndUniquePositions) {
  constexpr int kPerThread = 15;
  std::atomic<int> failures{0};

  RunInParallel([&](int thread) {
    Core core;
    if (core.Unlock(test_file, kPassword) != CoreStatus::kOk) {
      ++failures;
      return;
    }
    for (int i = 0; i < kPerThread; ++i) {
      const std::string name =
          "t" + std::to_string(thread) + "-" + std::to_string(i);
      if (core.AddAccount(MakeAccount(name, "p")) != CoreStatus::kOk) {
        ++failures;
      }
    }
  });
  ASSERT_EQ(failures.load(), 0);

  Core core;
  ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  ASSERT_EQ(core.Index().size(),
            static_cast<std::size_t>(kThreads * kPerThread));
  std::set<std::int64_t> positions;
  std::set<std::int64_t> ids;
  for (const AccountEntry& entry : core.Index()) {
    positions.insert(entry.position);
    ids.insert(entry.id);
  }
  EXPECT_EQ(positions.size(), core.Index().size());
  EXPECT_EQ(*positions.begin(), 0);
  EXPECT_EQ(*positions.rbegin(),
            static_cast<std::int64_t>(core.Index().size()) - 1);
  EXPECT_EQ(ids.size(), core.Index().size());
}

TEST_F(ConcurrencyTest, ConcurrentEditsNeverLoseAnUpdate) {
  std::int64_t id = -1;
  {
    Core core;
    ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
    ASSERT_EQ(core.AddAccount(MakeAccount("counter", "0"), &id),
              CoreStatus::kOk);
  }

  // Every thread increments the number stored in the password field with a
  // read-modify-write loop, retrying on kConflict. If an edit could ever
  // overwrite a newer one, increments would be lost.
  constexpr int kIncrements = 10;
  std::atomic<int> failures{0};
  RunInParallel([&](int /*thread*/) {
    Core core;
    if (core.Unlock(test_file, kPassword) != CoreStatus::kOk) {
      ++failures;
      return;
    }
    for (int done = 0; done < kIncrements;) {
      const auto account = core.GetAccount(id);
      if (!account.has_value()) {
        ++failures;
        return;
      }
      const int value =
          std::stoi(std::string(GetAccountField((*account)->password)));
      const std::int64_t revision = (*account)->revision;
      const CoreStatus status = core.EditAccount(
          id, revision, MakeAccount("counter", std::to_string(value + 1)));
      if (status == CoreStatus::kOk) {
        ++done;
      } else if (status != CoreStatus::kConflict) {
        ++failures;
        return;
      }
    }
  });
  ASSERT_EQ(failures.load(), 0);

  Core core;
  ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
  const auto account = core.GetAccount(id);
  ASSERT_TRUE(account.has_value());
  EXPECT_EQ(GetAccountField((*account)->password),
            std::to_string(kThreads * kIncrements));
  EXPECT_EQ((*account)->revision, kThreads * kIncrements);
}

TEST_F(ConcurrencyTest, ReordersAndReadsKeepTheListConsistent) {
  constexpr int kAccounts = 8;
  {
    Core core;
    ASSERT_EQ(core.Unlock(test_file, kPassword), CoreStatus::kOk);
    for (int i = 0; i < kAccounts; ++i) {
      ASSERT_EQ(core.AddAccount(MakeAccount("a" + std::to_string(i), "p")),
                CoreStatus::kOk);
    }
  }

  std::atomic<int> failures{0};
  RunInParallel([&](int thread) {
    Core core;
    if (core.Unlock(test_file, kPassword) != CoreStatus::kOk) {
      ++failures;
      return;
    }
    for (int i = 0; i < 20; ++i) {
      if (core.LoadIndex() != CoreStatus::kOk ||
          core.Index().size() != static_cast<std::size_t>(kAccounts)) {
        ++failures;
        return;
      }
      if (thread % 2 == 0) {
        // Move the first account to the end.
        const std::int64_t id = core.Index().front().id;
        if (core.ReorderAccount(id, kAccounts - 1) != CoreStatus::kOk) {
          ++failures;
        }
      } else {
        // Read any account: the index may be stale already, but an id
        // always names a real account here (nothing is removed).
        const std::int64_t id = core.Index()[i % kAccounts].id;
        if (!core.GetAccount(id).has_value()) {
          ++failures;
        }
      }
    }
  });
  ASSERT_EQ(failures.load(), 0);

  DB db;
  ASSERT_TRUE(db.Open(test_file));
  const auto index = db.GetAllIndex();
  ASSERT_TRUE(index.has_value());
  std::set<std::int64_t> positions;
  for (const AccountIndex& entry : *index) {
    positions.insert(entry.position);
  }
  EXPECT_EQ(positions.size(), static_cast<std::size_t>(kAccounts));
  EXPECT_EQ(*positions.rbegin(), kAccounts - 1);
}

TEST_F(ConcurrencyTest, PasswordChangeLocksOutEveryOtherSession) {
  std::atomic<int> locked{0};
  std::atomic<int> failures{0};
  std::atomic<bool> changed{false};
  // Last unexpected status, to explain a failure.
  std::atomic<CoreStatus> unexpected{CoreStatus::kOk};
  // Every session must be unlocked with the old password before the change.
  std::latch unlocked(kThreads);

  RunInParallel([&](int thread) {
    Core core;
    const CoreStatus unlock = core.Unlock(test_file, kPassword);
    unlocked.arrive_and_wait();
    if (unlock != CoreStatus::kOk) {
      unexpected = unlock;
      ++failures;
      return;
    }
    if (thread == 0) {
      const CoreStatus change =
          core.ChangeMasterPassword(kPassword, "new password");
      if (change != CoreStatus::kOk) {
        unexpected = change;
        ++failures;
      }
      changed = true;
      return;
    }
    // Keep writing until our session notices the change.
    for (int i = 0; i < 10000; ++i) {
      const CoreStatus status = core.AddAccount(MakeAccount("x", "p"));
      if (status == CoreStatus::kLocked) {
        ++locked;
        return;
      }
      if (status != CoreStatus::kOk) {
        unexpected = status;
        ++failures;
        return;
      }
      if (changed && core.IsOpen()) {
        // The change is committed; the next write must be refused.
        const CoreStatus next = core.AddAccount(MakeAccount("x", "p"));
        if (next == CoreStatus::kLocked) {
          ++locked;
        } else {
          unexpected = next;
          ++failures;
        }
        return;
      }
    }
    ++failures;
  });
  ASSERT_EQ(failures.load(), 0) << ToString(unexpected.load());
  EXPECT_EQ(locked.load(), kThreads - 1);

  // Every account in the vault, including those added concurrently with
  // the change, opens with the new password.
  Core core;
  ASSERT_EQ(core.Unlock(test_file, "new password"), CoreStatus::kOk);
  ASSERT_EQ(core.LoadIndex(), CoreStatus::kOk);
  EXPECT_EQ(core.Index().size(), core.Count().value());
}
