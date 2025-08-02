#include "datos.hh"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "cuenta.hh"
#include "set_rand.hh"

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

class DatosTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { SetRand(); }

  static void TearDownTestSuite() { std::remove(Datos::NOMBRE_ARCHIVO.c_str()); }

  inline static const std::string key = "clave_test";
  Datos datos_{key};
};

TEST_F(DatosTest, AgregarCuenta) {
  const std::string id = "id1";
  Cuenta c = CrearCuentaConId(id);
  datos_.AgregarCuentaConId(c);

  auto cuentas = datos_.cuentas();
  ASSERT_EQ(cuentas.size(), 1);
  EXPECT_EQ(cuentas[0].id(), id);
}

TEST_F(DatosTest, IndexCuentaExistente) {
  const std::string id = "id2";
  Cuenta c = CrearCuentaConId(id);
  datos_.AgregarCuentaConId(c);

  EXPECT_EQ(datos_.index(id), 0);
  EXPECT_EQ(datos_.index("noexiste"), -1);
}

TEST_F(DatosTest, ModificarCuenta) {
  const std::string id = "id3";
  Cuenta c1 = CrearCuentaConId(id);
  datos_.AgregarCuentaConId(c1);

  Cuenta c_mod = CrearCuentaConId(id);
  c_mod.SetContra("nueva_contra");
  datos_.ModificarCuenta(0, c_mod);

  auto cuentas = datos_.cuentas();
  ASSERT_EQ(cuentas.size(), 1);
  EXPECT_EQ(cuentas[0].contra(), "nueva_contra");
}

TEST_F(DatosTest, EliminarCuenta) {
  const std::string id[2] = {"id4", "id5"};
  Cuenta c1 = CrearCuentaConId(id[0]);
  Cuenta c2 = CrearCuentaConId(id[1]);
  datos_.AgregarCuentaConId(c1);
  datos_.AgregarCuentaConId(c2);

  datos_.EliminarCuenta(id[0]);

  ASSERT_EQ(datos_.cuentas().size(), 1);
  EXPECT_EQ(datos_.cuentas()[0].id(), id[1]);

  // Eliminar id inexistente no debe fallar
  datos_.EliminarCuenta("noexiste");
  EXPECT_EQ(datos_.cuentas().size(), 1);
}

TEST_F(DatosTest, GenerarIdUnico) {
  // Agrego varias cuentas con ids genéricos
  const size_t cant_cuentas = 100;
  for (int i = 0; i < cant_cuentas; ++i) {
    Cuenta c = CrearCuentaConId("id_gen");
    datos_.AgregarCuenta(c);
  }

  for (int i = 0; i < cant_cuentas; i++) {
    Cuenta c = datos_.cuentas()[i];
    datos_.EliminarCuenta(c.id());
    EXPECT_TRUE(IdUnico(datos_.cuentas(), c.id()));
    datos_.AgregarCuentaConId(c);
  }
}

// Tests básicos para Cargar y Guardar (depende implementación real)
TEST_F(DatosTest, GuardarYCargarCuentas) {
  const std::string id[2] = {"id7", "id8"};
  Cuenta c1 = CrearCuentaConId(id[0]);
  Cuenta c2 = CrearCuentaConId(id[1]);
  datos_.AgregarCuentaConId(c1);
  datos_.AgregarCuentaConId(c2);

  datos_.GuardarCuentas(key);
  Datos datos_nueva(key);
  datos_nueva.CargarCuentas(key);

  auto cuentas = datos_nueva.cuentas();
  EXPECT_GE(cuentas.size(), 2);

  // Verifico que al menos una cuenta tenga id7 o id8
  bool tiene_id7 = false, tiene_id8 = false;
  for (const auto& c : cuentas) {
    if (c.id() == id[0])
      tiene_id7 = true;
    if (c.id() == id[1])
      tiene_id8 = true;
  }
  EXPECT_TRUE(tiene_id7);
  EXPECT_TRUE(tiene_id8);
}
