#include "datos.hh"

#include "db.hh"

Datos::Datos(const std::string& key) { CargarCuentas(key); }
Datos::~Datos() {}

auto Datos::IdUnico(const std::string& id) const -> bool {
  for (int i = 0; i < cuentas_.size(); i++)
    if (cuentas_[i].id() == id) return false;
  return true;
}

auto Datos::GenerarIdUnico() const -> std::string {
  std::string str;
  do {
    int num = rand();
    str = std::string(reinterpret_cast<const char*>(&num), sizeof(int));
  } while (!IdUnico(str));
  return str;
}

auto Datos::index(const std::string& id) const -> int {
  for (int i = 0; i < cuentas_.size(); i++)
    if (cuentas_[i].id() == id) return i;
  return -1;
}

void Datos::AgregarCuenta(Cuenta& cuenta) {
  cuenta.SetId(GenerarIdUnico());
  cuentas_.push_back(cuenta);
}

void Datos::AgregarCuentaConId(Cuenta& cuenta) { cuentas_.push_back(cuenta); }

void Datos::ModificarCuenta(const size_t& index, const Cuenta& cuenta) {
  cuentas_[index] = cuenta;
}

void Datos::EliminarCuenta(const std::string& id) {
  const auto i = index(id);
  if (i != -1) cuentas_.erase(cuentas_.begin() + i);
}

void Datos::CargarCuentas(const std::string& key) {
  std::vector<DataBlock> datos = DB::Leer(Datos::kNombreArchivo);

  for (size_t i = 0; i < datos.size(); i += Cuenta::kCantAtributos) {
    std::array<DataBlock, Cuenta::kCantAtributos> c;

    for (int j = 0; j < Cuenta::kCantAtributos; j++) c[j] = datos[i + j];

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

  DB::Escribir(Datos::kNombreArchivo, datos);
}