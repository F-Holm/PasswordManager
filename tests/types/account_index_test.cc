#include "types/account_index.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "types/account_binary.h"
#include "types/account_size.h"

namespace {

bool IsDescriptionZeroed(const AccountIndex& index) {
  return std::all_of(index.description.begin(), index.description.end(),
                     [](std::byte b) { return b == std::byte{0}; });
}

TEST(AccountIndexTest, DefaultConstructor) {
  AccountIndex index;
  EXPECT_TRUE(IsDescriptionZeroed(index));
}

TEST(AccountIndexTest, PositionConstructor) {
  std::int64_t position = 27182;
  AccountIndex index(position);

  EXPECT_EQ(index.position, position);
  EXPECT_TRUE(IsDescriptionZeroed(index));
}

TEST(AccountIndexTest, ConstructorCopiesDescriptionCorrectly) {
  AccountBinary binary;
  for (size_t i = 0; i < AccountSize::kDescription; ++i) {
    binary.description[i] = static_cast<std::byte>(i + 1);
  }

  AccountIndex index(binary);

  EXPECT_EQ(index.position, -1);
  for (size_t i = 0; i < AccountSize::kDescription; ++i) {
    EXPECT_EQ(index.description[i], static_cast<std::byte>(i + 1))
        << "Mismatch at index " << i;
  }
}

TEST(AccountIndexTest, ConstructorCopiesDescriptionCorrectlyAndPosition) {
  AccountBinary binary;
  for (size_t i = 0; i < AccountSize::kDescription; ++i) {
    binary.description[i] = static_cast<std::byte>(i + 1);
  }

  std::int64_t position = 27182;
  AccountIndex index(binary, position);

  EXPECT_EQ(index.position, position);
  for (size_t i = 0; i < AccountSize::kDescription; ++i) {
    EXPECT_EQ(index.description[i], static_cast<std::byte>(i + 1))
        << "Mismatch at index " << i;
  }
}

TEST(AccountIndexTest, SetZeroClearsData) {
  AccountBinary binary;
  std::memset(binary.description.data(), 0xAA, AccountSize::kDescription);

  AccountIndex index(binary);
  ASSERT_FALSE(IsDescriptionZeroed(index));

  index.SetZero();

  EXPECT_TRUE(IsDescriptionZeroed(index));
}

TEST(AccountIndexTest, DestructorImplicitlyClearsData) {
  alignas(AccountIndex) char buffer[sizeof(AccountIndex)];

  {
    AccountBinary binary;
    std::memset(binary.description.data(), 0xBB, AccountSize::kDescription);

    AccountIndex* index = new (buffer) AccountIndex(binary);
    ASSERT_FALSE(IsDescriptionZeroed(*index));

    index->~AccountIndex();
  }

  const auto* raw_bytes = reinterpret_cast<const std::byte*>(buffer);
  bool all_zero = std::all_of(raw_bytes, raw_bytes + sizeof(AccountIndex),
                              [](std::byte b) { return b == std::byte{0}; });

  EXPECT_TRUE(all_zero) << "Memory was not zeroed out by the destructor";
}

}  // namespace
