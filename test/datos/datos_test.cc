#include "datos.hh"

#include "cuenta.hh"
#include "set_rand.hh"

#include <vector>
#include <string>

#include <gtest/gtest.h>

// Helper para crear cuenta simple con id único
Cuenta CrearCuentaConId(const std::string& id) {
    constexpr size_t k = Cuenta::kCantAtributos;
    auto datos = std::array<std::string, k>{};
    for (size_t i = 0; i < k; ++i) {
        datos[i] = "campo" + std::to_string(i);
    }
    Cuenta c(datos, "clave");
    c.SetId(id);
    return c;
}

auto IdUnico(const std::vector<Cuenta>& cuentas, const std::string& id) -> bool {
  for (int i = 0; i < cuentas.size(); i++)
    if (cuentas[i].id() == id)
      return false;
  return true;
}

auto GenIdUnico(const std::vector<Cuenta>& cuentas) -> std::string {
  std::string str;
  do {
    int num = rand();
    str = std::string(reinterpret_cast<const char*>(&num), sizeof(int));
  } while (!IdUnico(cuentas, str));
  return str;
}

class DatosTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        SetRand();
    }

    Datos datos_{"clave_test"};
};

TEST_F(DatosTest, AgregarCuenta) {
    Cuenta c = CrearCuentaConId("id1");
    datos_.AgrCuenta(c);

    auto cuentas = datos_.cuentas();
    ASSERT_EQ(cuentas.size(), 1);
    EXPECT_EQ(cuentas[0].id(), "id1");
}

TEST_F(DatosTest, IndexCuentaExistente) {
    Cuenta c = CrearCuentaConId("id2");
    datos_.AgrCuenta(c);

    EXPECT_EQ(datos_.index("id2"), 0);
    EXPECT_EQ(datos_.index("noexiste"), -1);
}

TEST_F(DatosTest, ModificarCuenta) {
    Cuenta c1 = CrearCuentaConId("id3");
    datos_.AgrCuenta(c1);

    Cuenta c_mod = CrearCuentaConId("id3");
    c_mod.SetContra("nueva_contra");
    datos_.ModCuenta(c_mod);

    auto cuentas = datos_.cuentas();
    ASSERT_EQ(cuentas.size(), 1);
    EXPECT_EQ(cuentas[0].contra(), "nueva_contra");
}

TEST_F(DatosTest, EliminarCuenta) {
    Cuenta c1 = CrearCuentaConId("id4");
    Cuenta c2 = CrearCuentaConId("id5");
    datos_.AgrCuenta(c1);
    datos_.AgrCuenta(c2);

    datos_.ElimCuenta("id4");

    auto cuentas = datos_.cuentas();
    ASSERT_EQ(cuentas.size(), 1);
    EXPECT_EQ(cuentas[0].id(), "id5");

    // Eliminar id inexistente no debe fallar
    datos_.ElimCuenta("noexiste");
    EXPECT_EQ(datos_.cuentas().size(), 1);
}

TEST_F(DatosTest, IdUnico) {
    Cuenta c1 = CrearCuentaConId("id6");
    datos_.AgrCuenta(c1);
    auto cuentas = datos_.cuentas();

    // id6 ya está agregado
    EXPECT_FALSE(IdUnico(cuentas, "id6"));
    // id nuevo
    EXPECT_TRUE(IdUnico(cuentas, "id_nuevo"));
}

TEST_F(DatosTest, GenIdUnico) {
    // Agrego varias cuentas con ids genéricos
    for (int i = 0; i < 5; ++i) {
        Cuenta c = CrearCuentaConId("id_gen_" + std::to_string(i));
        datos_.AgrCuenta(c);
    }
    auto cuentas = datos_.cuentas();
    
    std::string nuevo_id = GenIdUnico(cuentas);

    EXPECT_TRUE(IdUnico(cuentas, nuevo_id));
}

// Tests básicos para Cargar y Guardar (depende implementación real)
TEST_F(DatosTest, GuardarYCargarCuentas) {
    Cuenta c1 = CrearCuentaConId("id7");
    Cuenta c2 = CrearCuentaConId("id8");
    datos_.AgrCuenta(c1);
    datos_.AgrCuenta(c2);

    datos_.GuardarCuentas("clave_test");
    Datos datos_nueva("clave_test");
    datos_nueva.CargarCuentas("clave_test");

    auto cuentas = datos_nueva.cuentas();
    EXPECT_GE(cuentas.size(), 2);

    // Verifico que al menos una cuenta tenga id7 o id8
    bool tiene_id7 = false, tiene_id8 = false;
    for (const auto& c : cuentas) {
        if (c.id() == "id7") tiene_id7 = true;
        if (c.id() == "id8") tiene_id8 = true;
    }
    EXPECT_TRUE(tiene_id7);
    EXPECT_TRUE(tiene_id8);
}
