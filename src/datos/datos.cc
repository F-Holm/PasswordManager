#include "datos.hh"

#include <random>

using std::array;
using std::string;
using std::vector;

const string Datos::NOMBRE_ARCHIVO = "FEDERICO.HOLM";

Datos::Datos(const string& key) { CargarCuentas(key); }
Datos::~Datos() {}

auto Datos::idUnico(const string ID) const -> bool {
  for (int i = 0; i < cuentas.size(); i++)
    if (cuentas[i].getId() == ID)
      return false;
  return true;
}

inline void Datos::SetRand() {
  std::random_device rd;
  srand(static_cast<unsigned int>(std::time(nullptr)) ^ rd());
}

string Datos::generarIdUnico() const {
  string str;
  setRand();
  do {
    int num = rand();
    str = string(reinterpret_cast<const char *>(&num), sizeof(int));
  } while (!idUnico(str));
  return str;
}

int Datos::getIndex(const string ID) const {
  for (int i = 0; i < cuentas.size(); i++)
    if (cuentas[i].getId() == ID)
      return i;
}

vector<Cuenta> Datos::getCuentas() const { return cuentas; }

void Datos::setCuentas(const vector<Cuenta> &nuevasCuentas) {
  cuentas = nuevasCuentas;
}

void Datos::agregarCuenta(Cuenta &cuenta) {
  cuenta.setId(generarIdUnico());
  cuentas.push_back(cuenta);
}

void Datos::modificarCuenta(const Cuenta &cuenta) {
  cuentas[getIndex(cuenta.getId())] = cuenta;
}

void Datos::eliminarCuenta(const std::string id) {
  cuentas.erase(cuentas.begin() + getIndex(id));
}

void Datos::cargarCuentas(const string KEY) {
  vector<DataBlock> datos = DB::leer(Datos::NOMBRE_ARCHIVO);

  for (size_t i = 0; i < datos.size(); i += Cuenta::cantAtributos) {
    array<DataBlock, Cuenta::cantAtributos> c;

    for (int j = 0; j < Cuenta::cantAtributos; j++)
      c[j] = datos[i + j];

    cuentas.emplace_back(c, KEY);
  }
}

void Datos::guardarCuentas(const string KEY) {
  vector<DataBlock> datos;

  for (Cuenta cuenta : cuentas) {

    array<DataBlock, Cuenta::cantAtributos> arr;
    arr = cuenta.escribirDataBlocks(KEY);
    datos.insert(datos.end(), begin(arr), end(arr));
  }

  DB::escribir(Datos::NOMBRE_ARCHIVO, datos);
}