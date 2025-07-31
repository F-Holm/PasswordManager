#include "openssl_adapter.hh"

#include <string>

#include <gtest/gtest.h>

// Test que verifica que Hash256 genera siempre el mismo resultado para la misma entrada
TEST(OpenSslAdapterTest, Hash256_Determinismo) {
    std::string input = "mensaje de prueba";
    std::string hash1 = OpenSslAdapter::Hash256(input);
    std::string hash2 = OpenSslAdapter::Hash256(input);

    EXPECT_EQ(hash1, hash2);
    EXPECT_EQ(hash1.size(), 256/8);  // SHA-256 en hex tiene 64 caracteres
}

// Test que verifica que Hash256_x devuelve exactamente len_rta caracteres
TEST(OpenSslAdapterTest, Hash256_X_LargoCorrecto) {
    std::string input = "dato";
    size_t len = 16;
    std::string hash = OpenSslAdapter::Hash256_x(input, len);

    EXPECT_EQ(hash.size(), len);
}

// Test que verifica que encriptar y luego desencriptar devuelve el mismo texto
TEST(OpenSslAdapterTest, EncriptarDesencriptar_Consistencia) {
    std::string texto_original = "este texto es confidencial";
    std::string clave = "clave-secreta-de-16"; // asumimos tamaño válido para AES
    std::string tag;

    std::string cifrado = OpenSslAdapter::Encriptar(texto_original, clave, tag);
    std::string descifrado = OpenSslAdapter::Desencriptar(cifrado, clave, tag);

    EXPECT_EQ(descifrado, texto_original);
}

// Test que cambia la clave y espera que no se pueda descifrar (o dé texto incorrecto)
TEST(OpenSslAdapterTest, Desencriptar_ClaveErronea) {
    std::string texto_original = "datos ultra secretos";
    std::string clave_correcta = "clave123456789012";
    std::string clave_erronea = "clave-incorrecta123";
    std::string tag;

    std::string cifrado = OpenSslAdapter::Encriptar(texto_original, clave_correcta, tag);
    std::string descifrado = OpenSslAdapter::Desencriptar(cifrado, clave_erronea, tag);

    // Puede fallar con excepción, string vacío o basura, depende de implementación
    EXPECT_NE(descifrado, texto_original);
}
