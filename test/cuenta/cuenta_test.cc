#include "cuenta.hh"

#include "data_block.hh"

#include <array>
#include <string>

#include <gtest/gtest.h>

// Helper para crear DataBlock desde string simple
DataBlock CrearDB(const std::string& s) {
    return DataBlock(s);
}

// Helper para crear array<string> con contenido simple
template<size_t N>
std::array<std::string, N> CrearArrayStr(const std::string& base) {
    std::array<std::string, N> arr;
    for (size_t i = 0; i < N; ++i) {
        arr[i] = base + std::to_string(i);
    }
    return arr;
}

// Test constructor con std::array<DataBlock, kCantAtributos>
TEST(CuentaTest, ConstructorDataBlock) {
    std::array<DataBlock, Cuenta::kCantAtributos> datos;
    for (size_t i = 0; i < Cuenta::kCantAtributos; ++i) {
        datos[i] = CrearDB("dato" + std::to_string(i));
    }
    Cuenta c(datos, "clave123");

    EXPECT_EQ(c.id(), "");
    EXPECT_EQ(c.contra(), "");
}

// Test constructor con std::array<string, kCantAtributos>
TEST(CuentaTest, ConstructorStringArray) {
    auto arr = CrearArrayStr<Cuenta::kCantAtributos>("campo");
    Cuenta c(arr, "clave456");

    EXPECT_EQ(c.id(), "");
    EXPECT_EQ(c.nombre_usuario(), "");
}

// Test constructor con mitad menos uno de strings
TEST(CuentaTest, ConstructorMitadMenosUno) {
    constexpr size_t n = (Cuenta::kCantAtributos / 2) - 1;
    auto arr = CrearArrayStr<n>("mitad");
    Cuenta c(arr);

    EXPECT_EQ(c.descripcion(), "");
    EXPECT_EQ(c.extra(), "");
}

// Test setters y getters básicos
TEST(CuentaTest, SettersGetters) {
    Cuenta c = Cuenta(CrearArrayStr<Cuenta::kCantAtributos>("x"), "clave");

    c.SetId("id1");
    c.SetDescripcion("desc1");
    c.SetEmail("email1");
    c.SetNombreUsuario("nom1");
    c.SetContra("contra1");
    c.SetExtra("extra1");

    EXPECT_EQ(c.id(), "id1");
    EXPECT_EQ(c.descripcion(), "desc1");
    EXPECT_EQ(c.email(), "email1");
    EXPECT_EQ(c.nombre_usuario(), "nom1");
    EXPECT_EQ(c.contra(), "contra1");
    EXPECT_EQ(c.extra(), "extra1");
}

// Test operador= (asignación)
TEST(CuentaTest, OperadorAsignacion) {
    Cuenta a = Cuenta(CrearArrayStr<Cuenta::kCantAtributos>("a"), "clave");
    Cuenta b = Cuenta(CrearArrayStr<Cuenta::kCantAtributos>("b"), "clave2");

    b = a;

    EXPECT_EQ(b.id(), a.id());
    EXPECT_EQ(b.contra(), a.contra());
}

// Test EscribirDataBlocks devuelve array con kCantAtributos elementos
TEST(CuentaTest, EscribirDataBlocks) {
    Cuenta c = Cuenta(CrearArrayStr<Cuenta::kCantAtributos>("x"), "clave");
    auto arr = c.EscribirDataBlocks("clave");

    EXPECT_EQ(arr.size(), Cuenta::kCantAtributos);
    // Opcional: verificar que los DataBlock no estén vacíos (según implementación)
    for (const auto& db : arr) {
        EXPECT_GT(db.largo, 0);
        EXPECT_NE(db.str, nullptr);
    }
}

// Test Encriptar y Desencriptar
TEST(CuentaTest, EncriptarDesencriptar) {
    Cuenta c = Cuenta(CrearArrayStr<Cuenta::kCantAtributos>("dato"), "clave");

    c.SetContra("pass1234");
    c.Encriptar("clave");
    std::string contra_encriptada = c.contra();
    EXPECT_NE(contra_encriptada, "pass1234");  // debería cambiar

    c.Desencriptar("clave");
    EXPECT_EQ(c.contra(), "pass1234");
}
