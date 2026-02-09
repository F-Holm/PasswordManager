#include "seguridad.hh"

#include <gtest/gtest.h>

#include <string>

// Test que verifica que Hash256 genera siempre el mismo resultado para la misma
// entrada
TEST(SeguridadTest, Hash256_Determinismo) {
  std::string input = "mensaje de prueba";
  std::string hash1 = Seguridad::Hash256(input);
  std::string hash2 = Seguridad::Hash256(input);

  EXPECT_EQ(hash1, hash2);
  EXPECT_EQ(hash1.size(), 256 / 8);
}

// Test que verifica que Hash256_x devuelve exactamente len_rta caracteres
TEST(SeguridadTest, Hash256_X_LargoCorrecto) {
  std::string input = "dato";
  size_t len = 16;
  std::string hash = Seguridad::Hash256_x(input, len);

  EXPECT_EQ(hash.size(), len);
}

// Test que encripta y luego desencripta correctamente
TEST(SeguridadTest, EncriptarDesencriptar_Consistencia) {
  std::string texto_original = "este texto es confidencial";
  std::string clave = "clave-secreta-de-16";  // tamaño válido para AES
  std::string tag;

  std::string cifrado = Seguridad::Encriptar(texto_original, clave, tag);
  std::string descifrado = Seguridad::Desencriptar(cifrado, clave, tag);

  EXPECT_EQ(descifrado, texto_original);
}

// Test que usa clave incorrecta para desencriptar
TEST(SeguridadTest, Desencriptar_ClaveErronea) {
  std::string texto_original = "datos ultra secretos";
  std::string clave_correcta = "clave123456789012";
  std::string clave_erronea = "clave-incorrecta123";
  std::string tag;

  std::string cifrado =
      Seguridad::Encriptar(texto_original, clave_correcta, tag);
  std::string descifrado = Seguridad::Desencriptar(cifrado, clave_erronea, tag);

  // Puede fallar con string vacío, basura o lanzar excepción (según
  // implementación)
  EXPECT_NE(descifrado, texto_original);
}
