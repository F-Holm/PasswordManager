#include "datos.hh"

#include "utils/set_rand.hh"

using std::array;
using std::string;
using std::vector;

const string Datos::NOMBRE_ARCHIVO = "FEDERICO.HOLM";

Datos::Datos(const string& key) { CargarCuentas(key); }
Datos::~Datos() {}

auto Datos::IdUnico(const string& id) const -> bool {
  for (int i = 0; i < cuentas.size(); i++)
    if (cuentas[i].getId() == id)
      return false;
  return true;
}

string Datos::GenIdUnico() const {
  string str;
  SetRand();
  do {
    int num = rand();
    str = string(reinterpret_cast<const char*>(&num), sizeof(int));
  } while (!IdUnico(str));
  return str;
}

auto Datos::index(const string& id) const -> int {
  for (int i = 0; i < cuentas.size(); i++)
    if (cuentas[i].getId() == id)
      return i;
}

void Datos::AgrCuenta(Cuenta &cuenta) {
  cuenta.SetId(GenIdUnico());
  cuentas.push_back(cuenta);
}

void Datos::ModCuenta(const Cuenta &cuenta) {
  cuentas[getIndex(cuenta.getId())] = cuenta;
}

void Datos::ElimCuenta(const std::string& id) {
  cuentas.erase(cuentas.begin() + index(id));
}

void Datos::CargarCuentas(const string& key) {
  vector<DataBlock> datos = DB::Leer(Datos::NOMBRE_ARCHIVO);

  for (size_t i = 0; i < datos.size(); i += Cuenta::cantAtributos) {
    array<DataBlock, Cuenta::cantAtributos> c;

    for (int j = 0; j < Cuenta::cantAtributos; j++)
      c[j] = datos[i + j];

    cuentas.emplace_back(c, key);
  }
}

void Datos::GuardarCuentas(const string& key) {
  vector<DataBlock> datos;

  for (Cuenta cuenta : cuentas) {
    array<DataBlock, Cuenta::kCantAtri> arr;
    arr = cuenta.EscribirDataBlocks(key);
    datos.insert(datos.end(), begin(arr), end(arr));
  }

  DB::Escribir(Datos::NOMBRE_ARCHIVO, datos);
}