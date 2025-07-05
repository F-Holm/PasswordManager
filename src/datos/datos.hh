#pragma once

#include <vector>
#include <string>

#include "cuenta.hh"

class Datos {
public:
  Datos(const std::string &key);
  ~Datos();

  const static std::string NOMBRE_ARCHIVO;

  auto GetIndex(const std::string& id) const -> int;
  auto centas() const -> std::vector<Cuenta> { return cuentas_; };

  void AgregarCuenta(Cuenta &cuenta);
  void ModificarCuenta(const Cuenta &cuenta);
  void EliminarCuenta(const std::string &id);

  void CargarCuentas(const std::string &key);
  void GuardarCuentas(const std::string &key);

  private:
  std::vector<Cuenta> cuentas_;

  void SetCuentas(const std::vector<Cuenta> &cuentas);

  auto IdUnico(const std::string& id) const -> bool;
  auto GenIdUnico() const -> std::string;
  inline static void SetRand();
};