#include "account/account_view.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>

#include "account/account_size.h"

namespace {

TEST(AccountViewTest, ConstructorCorrectlyMapsOffsets) {
  AccountBinary binary;

  binary.description[0] = std::byte{0xDE};

  binary.data[0] = std::byte{0xE1};
  binary.data[AccountSize::kEmail] = std::byte{0x02};
  binary.data[AccountSize::kEmail + AccountSize::kUsername] = std::byte{0x03};

  AccountView view(binary);

  EXPECT_EQ(view.description.data(), binary.description);
  EXPECT_EQ(view.description[0], std::byte{0xDE});
  EXPECT_EQ(view.description.size(), AccountSize::kDescription);

  EXPECT_EQ(view.email.data(), binary.data);
  EXPECT_EQ(view.email[0], std::byte{0xE1});
  EXPECT_EQ(view.email.size(), AccountSize::kEmail);

  EXPECT_EQ(view.user_name.data(), binary.data + AccountSize::kEmail);
  EXPECT_EQ(view.user_name[0], std::byte{0x02});

  EXPECT_EQ(view.password.data(),
            binary.data + AccountSize::kEmail + AccountSize::kUsername);
  EXPECT_EQ(view.password[0], std::byte{0x03});
}

TEST(AccountViewTest, ClearSetsSpansToNull) {
  AccountBinary binary;
  AccountView view(binary);

  ASSERT_NE(view.description.data(), nullptr);

  view.Clear();

  EXPECT_TRUE(view.description.empty());
  EXPECT_EQ(view.description.data(), nullptr);

  EXPECT_TRUE(view.email.empty());
  EXPECT_EQ(view.email.data(), nullptr);

  EXPECT_TRUE(view.password.empty());
  EXPECT_EQ(view.password.data(), nullptr);
}

TEST(AccountViewTest, DestructorInvalidatesViewButPreservesBinary) {
  AccountBinary binary;
  std::memset(binary.description, 0x42, AccountSize::kDescription);

  {
    AccountView view(binary);
    EXPECT_EQ(view.description[0], std::byte{0x42});
  }

  EXPECT_EQ(binary.description[0], std::byte{0x42});
}

}  // namespace