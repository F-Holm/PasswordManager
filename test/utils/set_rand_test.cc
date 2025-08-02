#include "set_rand.hh"

#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <random>

TEST(SetRandTest, CambiaSecuenciaRand) {
  SetRand();
  int valores_1[5];
  for (int i = 0; i < 5; ++i) {
    valores_1[i] = rand();
  }

  SetRand();
  int valores_2[5];
  for (int i = 0; i < 5; ++i) {
    valores_2[i] = rand();
  }

  // Verificamos que las dos secuencias no sean iguales (probablemente distintas)
  bool iguales = true;
  for (int i = 0; i < 5; ++i) {
    if (valores_1[i] != valores_2[i]) {
      iguales = false;
      break;
    }
  }

  EXPECT_FALSE(iguales);
}
