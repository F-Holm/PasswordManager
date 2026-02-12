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

  std::memset(account.description, 0xAA, sizeof(account.description));
  std::memset(account.data, 0xBB, sizeof(account.data));
  std::memset(account.iv_description, 0xCC, sizeof(account.iv_description));
  std::memset(account.iv_data, 0xDD, sizeof(account.iv_data));
  std::memset(account.tag_description, 0xEE, sizeof(account.tag_description));
  std::memset(account.tag_data, 0xFF, sizeof(account.tag_data));

  account.SetZero();

  EXPECT_TRUE(IsBufferZeroed(account.description, sizeof(account.description)));
  EXPECT_TRUE(IsBufferZeroed(account.data, sizeof(account.data)));
  EXPECT_TRUE(
      IsBufferZeroed(account.iv_description, sizeof(account.iv_description)));
  EXPECT_TRUE(IsBufferZeroed(account.iv_data, sizeof(account.iv_data)));
  EXPECT_TRUE(
      IsBufferZeroed(account.tag_description, sizeof(account.tag_description)));
  EXPECT_TRUE(IsBufferZeroed(account.tag_data, sizeof(account.tag_data)));
}

TEST(AccountBinaryTest, DestructorImplicitlyCallsSetZero) {
  alignas(AccountBinary) char raw_mem[sizeof(AccountBinary)];
  {
    AccountBinary* account = new (raw_mem) AccountBinary();

    std::memset(account->data, 0x42, sizeof(account->data));
    ASSERT_FALSE(IsBufferZeroed(account->data, sizeof(account->data)));

    account->~AccountBinary();
  }

  EXPECT_TRUE(IsBufferZeroed(reinterpret_cast<const std::byte*>(raw_mem),
                             sizeof(raw_mem)));
}

}  // namespace
