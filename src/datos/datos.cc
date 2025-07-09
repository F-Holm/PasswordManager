#include "datos.hh"

using std::array;
using std::string;
using std::vector;

const string Datos::NOMBRE_ARCHIVO = "FEDERICO.HOLM";

Datos::Datos(const string& key) { CargarCuentas(key); }
Datos::~Datos() {}

auto Datos::IdUnico(const string& id) const -> bool {
  for (int i = 0; i < cuentas_.size(); i++)
    if (cuentas_[i].id() == id)
      return false;
  return true;
}

auto Datos::GenIdUnico() const -> string {
  string str;
  do {
    int num = rand();
    str = string(reinterpret_cast<const char*>(&num), sizeof(int));
  } while (!IdUnico(str));
  return str;
}

auto Datos::index(const string& id) const -> int {
  for (int i = 0; i < cuentas_.size(); i++)
    if (cuentas_[i].id() == id)
      return i;
}

void Datos::AgrCuenta(Cuenta &cuenta) {
  cuenta.SetId(GenIdUnico());
  cuentas_.push_back(cuenta);
}

void Datos::ModCuenta(const Cuenta &cuenta) {
  cuentas_[index(cuenta.id())] = cuenta;
}

void Datos::ElimCuenta(const std::string& id) {
  cuentas_.erase(cuentas_.begin() + index(id));
}

void Datos::CargarCuentas(const string& key) {
  vector<DataBlock> datos = DB::Leer(Datos::NOMBRE_ARCHIVO);

  for (size_t i = 0; i < datos.size(); i += Cuenta::kCantAtri) {
    array<DataBlock, Cuenta::kCantAtri> c;

    for (int j = 0; j < Cuenta::kCantAtri; j++)
      c[j] = datos[i + j];

    cuentas_.emplace_back(c, key);
  }
}

void Datos::GuardarCuentas(const string& key) {
  vector<DataBlock> datos;

  for (Cuenta cuenta : cuentas_) {
    array<DataBlock, Cuenta::kCantAtri> arr;
    arr = cuenta.EscribirDataBlocks(key);
    datos.insert(datos.end(), begin(arr), end(arr));
  }

  DB::Escribir(Datos::NOMBRE_ARCHIVO, datos);
}