#pragma once

#include <vector>
#include <string>

#include "cuenta.hh"

class Datos {
private:
  std::vector<Cuenta> cuentas;

  bool idUnico(const std::string id) const;
  std::string generarIdUnico() const;
  static void setRand();

public:
  Datos(const std::string &key);
  ~Datos();

  const static std::string NOMBRE_ARCHIVO;

  int GetIndex(const std::string ID) const;
  std::vector<Cuenta> GetCuentas() const;
  void SetCuentas(const std::vector<Cuenta> &cuentas);

  void AgregarCuenta(Cuenta &cuenta);
  void ModificarCuenta(const Cuenta &cuenta);
  void EliminarCuenta(const std::string &id);

  void CargarCuentas(const std::string &key);
  void GuardarCuentas(const std::string &key);
};