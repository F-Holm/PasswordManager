#pragma once

#include <vector>
#include <string>

#include "cuenta.hh"

class Datos {
public:
  Datos(const std::string &key);
  ~Datos();

  inline static const std::string NOMBRE_ARCHIVO = "FEDERICO.HOLM";

  auto index(const std::string& id) const -> int;
  auto cuentas() const -> std::vector<Cuenta> { return cuentas_; };

  void AgrCuenta(Cuenta &cuenta);
  void ModCuenta(const Cuenta &cuenta);
  void ElimCuenta(const std::string &id);

  void CargarCuentas(const std::string &key);
  void GuardarCuentas(const std::string &key);

  private:
  std::vector<Cuenta> cuentas_;

  void SetCuentas(const std::vector<Cuenta> &cuentas) { cuentas_ = cuentas;}

  auto IdUnico(const std::string& id) const -> bool;
  auto GenIdUnico() const -> std::string;
};