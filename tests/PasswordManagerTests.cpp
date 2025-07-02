#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include "Seguridad.h"

TEST(SHATest, Hashing) { EXPECT_NE("GTest", Seguridad::hash256("GTest")); }

TEST(SHATest, HashesAreUnique) {
  EXPECT_NE(Seguridad::hash256("GTest"), Seguridad::hash256("gTEST"));
}

TEST(SHATest, SHALength) {
  EXPECT_EQ(256 / 8, Seguridad::hash256("GTest").size());
}

TEST(AESTest, EncDecTest) {
  std::string tag;
  const std::string KEY = "KEY";
  std::string str = "GTest";
  std::string enc_str = Seguridad::encriptar(str, KEY, tag);
  std::string dec_str = Seguridad::desencriptar(enc_str, KEY, tag);

  EXPECT_NE(str, enc_str);
  EXPECT_EQ(str, dec_str);
}