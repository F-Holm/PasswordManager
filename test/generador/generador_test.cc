#include "generador.hh"

#include <gtest/gtest.h>

#include <regex>

// Test que verifica el largo de la contraseña
TEST(GenerarContraTest, LargoCorrecto) {
  for (int i = 0; i <= 100; ++i) {
    std::string contra =
        Generador::GenerarContra(i, Generador::TipoContra::COMP);
    EXPECT_EQ(contra.size(), i);
  }
}

// Test ALFA (mayúsculas + minúsculas)
TEST(GenerarContraTest, SoloLetras) {
  std::string contra =
      Generador::GenerarContra(50, Generador::TipoContra::ALFA);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Za-z]+$")));
}

// Test ALFA_MAY (solo mayúsculas)
TEST(GenerarContraTest, SoloMayusculas) {
  std::string contra =
      Generador::GenerarContra(30, Generador::TipoContra::ALFA_MAY);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Z]+$")));
}

// Test ALFA_MIN (solo minúsculas)
TEST(GenerarContraTest, SoloMinusculas) {
  std::string contra =
      Generador::GenerarContra(30, Generador::TipoContra::ALFA_MIN);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[a-z]+$")));
}

// Test NUM (solo números)
TEST(GenerarContraTest, SoloNumeros) {
  std::string contra = Generador::GenerarContra(20, Generador::TipoContra::NUM);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[0-9]+$")));
}

// Test ALFA_NUM (letras y números)
TEST(GenerarContraTest, LetrasYNumeros) {
  std::string contra =
      Generador::GenerarContra(40, Generador::TipoContra::ALFA_NUM);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Za-z0-9]+$")));
}

// Test ALFA_NUM_MAY (mayúsculas + números)
TEST(GenerarContraTest, MayusculasYNumeros) {
  std::string contra =
      Generador::GenerarContra(40, Generador::TipoContra::ALFA_NUM_MAY);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Z0-9]+$")));
}

// Test ALFA_NUM_MIN (minúsculas + números)
TEST(GenerarContraTest, MinusculasYNumeros) {
  std::string contra =
      Generador::GenerarContra(40, Generador::TipoContra::ALFA_NUM_MIN);
  EXPECT_TRUE(std::regex_match(contra, std::regex("^[a-z0-9]+$")));
}