#pragma once

#include <string>
#include <vector>

#include "cuenta.hh"

class Datos {
 public:
  Datos(const std::string &key);
  ~Datos();

  inline static const std::string kNombreArchivo = "FEDERICO.HOLM";

  auto index(const std::string &id) const -> int;
  auto cuentas() const -> std::vector<Cuenta> { return cuentas_; };

  void AgregarCuenta(Cuenta &cuenta);
  void AgregarCuentaConId(Cuenta &cuenta);
  void ModificarCuenta(const size_t &index, const Cuenta &cuenta);
  void EliminarCuenta(const std::string &id);

  void CargarCuentas(const std::string &key);
  void GuardarCuentas(const std::string &key);

 private:
  std::vector<Cuenta> cuentas_;

  void SetCuentas(const std::vector<Cuenta> &cuentas) { cuentas_ = cuentas; }

  auto IdUnico(const std::string &id) const -> bool;
  auto GenerarIdUnico() const -> std::string;
};