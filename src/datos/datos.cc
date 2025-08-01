#include "datos.hh"

#include "db.hh"

Datos::Datos(const std::string& key) { CargarCuentas(key); }
Datos::~Datos() {}

auto Datos::IdUnico(const std::string& id) const -> bool {
  for (int i = 0; i < cuentas_.size(); i++)
    if (cuentas_[i].id() == id)
      return false;
  return true;
}

auto Datos::GenIdUnico() const -> std::string {
  std::string str;
  do {
    int num = rand();
    str = std::string(reinterpret_cast<const char*>(&num), sizeof(int));
  } while (!IdUnico(str));
  return str;
}

auto Datos::index(const std::string& id) const -> int {
  for (int i = 0; i < cuentas_.size(); i++)
    if (cuentas_[i].id() == id)
      return i;
  return -1;
}

void Datos::AgrCuenta(Cuenta &cuenta) {
  cuenta.SetId(GenIdUnico());
  cuentas_.push_back(cuenta);
}

void Datos::AgrCuentaConId(Cuenta &cuenta) {
  cuentas_.push_back(cuenta);
}

void Datos::ModCuenta(const size_t& index, const Cuenta& cuenta) {
  cuentas_[index] = cuenta;
}

void Datos::ElimCuenta(const std::string& id) {
  cuentas_.erase(cuentas_.begin() + index(id));
}

void Datos::CargarCuentas(const std::string& key) {
  std::vector<DataBlock> datos = DB::Leer(Datos::NOMBRE_ARCHIVO);

  for (size_t i = 0; i < datos.size(); i += Cuenta::kCantAtributos) {
    std::array<DataBlock, Cuenta::kCantAtributos> c;

    for (int j = 0; j < Cuenta::kCantAtributos; j++)
      c[j] = datos[i + j];

    cuentas_.emplace_back(c, key);
  }
}

void Datos::GuardarCuentas(const std::string& key) {
  std::vector<DataBlock> datos;

  for (Cuenta& cuenta : cuentas_) {
    std::array<DataBlock, Cuenta::kCantAtributos> arr;
    arr = cuenta.EscribirDataBlocks(key);
    datos.insert(datos.end(), begin(arr), end(arr));
  }

  DB::Escribir(Datos::NOMBRE_ARCHIVO, datos);
}