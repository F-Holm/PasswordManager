#include "generador.hh"

#include <regex>

#include <gtest/gtest.h>

// Test que verifica el largo de la contraseña
TEST(GenerarContraTest, LargoCorrecto) {
    for (int i = 1; i <= 100; ++i) {
        std::string contra = Gen::GenerarContra(i, Gen::TipoContra::COMP);
        EXPECT_EQ(contra.size(), i);
    }
}

// Test ALFA (mayúsculas + minúsculas)
TEST(GenerarContraTest, SoloLetras) {
    std::string contra = Gen::GenerarContra(50, Gen::TipoContra::ALFA);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Za-z]+$")));
}

// Test ALFA_MAY (solo mayúsculas)
TEST(GenerarContraTest, SoloMayusculas) {
    std::string contra = Gen::GenerarContra(30, Gen::TipoContra::ALFA_MAY);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Z]+$")));
}

// Test ALFA_MIN (solo minúsculas)
TEST(GenerarContraTest, SoloMinusculas) {
    std::string contra = Gen::GenerarContra(30, Gen::TipoContra::ALFA_MIN);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[a-z]+$")));
}

// Test NUM (solo números)
TEST(GenerarContraTest, SoloNumeros) {
    std::string contra = Gen::GenerarContra(20, Gen::TipoContra::NUM);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[0-9]+$")));
}

// Test ALFA_NUM (letras y números)
TEST(GenerarContraTest, LetrasYNumeros) {
    std::string contra = Gen::GenerarContra(40, Gen::TipoContra::ALFA_NUM);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Za-z0-9]+$")));
}

// Test ALFA_NUM_MAY (mayúsculas + números)
TEST(GenerarContraTest, MayusculasYNumeros) {
    std::string contra = Gen::GenerarContra(40, Gen::TipoContra::ALFA_NUM_MAY);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[A-Z0-9]+$")));
}

// Test ALFA_NUM_MIN (minúsculas + números)
TEST(GenerarContraTest, MinusculasYNumeros) {
    std::string contra = Gen::GenerarContra(40, Gen::TipoContra::ALFA_NUM_MIN);
    EXPECT_TRUE(std::regex_match(contra, std::regex("^[a-z0-9]+$")));
}