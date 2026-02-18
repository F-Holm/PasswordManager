#include "account/account_binary.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace {

bool IsBufferZeroed(const std::byte* data, size_t size) {
  return std::all_of(data, data + size,
                     [](std::byte b) { return b == std::byte{0}; });
}

TEST(AccountBinaryTest, SetZeroClearsAllFields) {
  AccountBinary account;

  std::memset(account.description.data(), 0xAA, account.description.size());
  std::memset(account.data.data(), 0xBB, account.data.size());
  std::memset(account.iv_description.data(), 0xCC,
              account.iv_description.size());
  std::memset(account.iv_data.data(), 0xDD, account.iv_data.size());
  std::memset(account.tag_description.data(), 0xEE,
              account.tag_description.size());
  std::memset(account.tag_data.data(), 0xFF, account.tag_data.size());

  account.SetZero();

  EXPECT_TRUE(
      IsBufferZeroed(account.description.data(), account.description.size()));
  EXPECT_TRUE(IsBufferZeroed(account.data.data(), account.data.size()));
  EXPECT_TRUE(IsBufferZeroed(account.iv_description.data(),
                             account.iv_description.size()));
  EXPECT_TRUE(IsBufferZeroed(account.iv_data.data(), account.iv_data.size()));
  EXPECT_TRUE(IsBufferZeroed(account.tag_description.data(),
                             account.tag_description.size()));
  EXPECT_TRUE(IsBufferZeroed(account.tag_data.data(), account.tag_data.size()));
}

TEST(AccountBinaryTest, DestructorImplicitlyCallsSetZero) {
  alignas(AccountBinary) char raw_mem[sizeof(AccountBinary)];
  {
    AccountBinary* account = new (raw_mem) AccountBinary();

    std::memset(account->data.data(), 0x42, account->data.size());
    ASSERT_FALSE(IsBufferZeroed(account->data.data(), account->data.size()));

    account->~AccountBinary();
  }

  EXPECT_TRUE(IsBufferZeroed(reinterpret_cast<const std::byte*>(raw_mem),
                             sizeof(raw_mem)));
}

}  // namespace
