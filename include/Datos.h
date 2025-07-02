#pragma once

#include "Cuenta.h"

class Datos {
private:
  std::vector<Cuenta> cuentas;

  bool idUnico(const std::string id) const;
  std::string generarIdUnico() const;
  static void setRand();

public:
  Datos(const std::string KEY);
  ~Datos();

  const static std::string NOMBRE_ARCHIVO;

  int getIndex(const std::string ID) const;
  std::vector<Cuenta> getCuentas() const;
  void setCuentas(const std::vector<Cuenta> &cuentas);

  void agregarCuenta(Cuenta &cuenta);
  void modificarCuenta(const Cuenta &cuenta);
  void eliminarCuenta(const std::string id);

  void cargarCuentas(const std::string KEY);
  void guardarCuentas(const std::string KEY);
};