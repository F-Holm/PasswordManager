#include "utils/set_rand.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <random>

TEST(SetRandTest, ChangesRandSequence) {
  constexpr size_t kAttempts = 100;

  SetRand();
  int values_1[kAttempts];
  for (int i = 0; i < kAttempts; ++i) {
    values_1[i] = rand();
  }

  SetRand();
  int values_2[kAttempts];
  for (int i = 0; i < kAttempts; ++i) {
    values_2[i] = rand();
  }

  // We verify that the two sequences are not equal (very likely different)
  bool equal = true;
  for (int i = 0; i < kAttempts; ++i) {
    if (values_1[i] != values_2[i]) {
      equal = false;
      break;
    }
  }

  EXPECT_FALSE(equal);
}
