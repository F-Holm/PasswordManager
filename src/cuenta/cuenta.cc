#include "cuenta.hh"

#include "seguridad.hh"
#include "data_block.hh"

using std::array;
using std::string;

// Constructores, destructores y operators
Cuenta::Cuenta(array<DataBlock, Cuenta::kCantAtri> datos, const string &key) {
  id_ = string(reinterpret_cast<const char *>(datos[0].str), datos[0].largo);
  desc_ = string(reinterpret_cast<const char *>(datos[1].str), datos[1].largo);
  email_ = string(reinterpret_cast<const char *>(datos[2].str), datos[2].largo);
  nom_ = string(reinterpret_cast<const char *>(datos[3].str), datos[3].largo);
  contra_ = string(reinterpret_cast<const char *>(datos[4].str), datos[4].largo);
  extra_ = string(reinterpret_cast<const char *>(datos[5].str), datos[5].largo);

  id_t_ = string(reinterpret_cast<const char *>(datos[6].str), datos[6].largo);
  desc_t_ = string(reinterpret_cast<const char *>(datos[7].str), datos[7].largo);
  email_t_ = string(reinterpret_cast<const char *>(datos[8].str), datos[8].largo);
  nom_t_ = string(reinterpret_cast<const char *>(datos[9].str), datos[9].largo);
  contra_t_ = string(reinterpret_cast<const char *>(datos[10].str), datos[10].largo);
  extra_t_ = string(reinterpret_cast<const char *>(datos[11].str), datos[11].largo);

  Desencriptar(key);
}

Cuenta::Cuenta(array<string, Cuenta::kCantAtri> datos, const string &key) {
  id_ = datos[0];
  desc_ = datos[1];
  email_ = datos[2];
  nom_ = datos[3];
  contra_ = datos[4];
  extra_ = datos[5];

  id_t_ = datos[6];
  desc_t_ = datos[7];
  email_t_ = datos[8];
  nom_t_ = datos[9];
  contra_t_ = datos[10];
  extra_t_ = datos[11];

  Desencriptar(key);
}

Cuenta::Cuenta(array<string, (Cuenta::kCantAtri / 2) - 1> datos) {
  id_ = "";
  desc_ = datos[0];
  email_ = datos[1];
  nom_ = datos[2];
  contra_ = datos[3];
  extra_ = datos[4];

  id_t_ = "";
  desc_t_ = "";
  email_t_ = "";
  nom_t_ = "";
  contra_t_ = "";
  extra_t_ = "";
}

Cuenta::~Cuenta() {}

auto Cuenta::operator=(const Cuenta &other) -> Cuenta & {
  if (this != &other) {
    id_ = other.id_;
    desc_ = other.desc_;
    email_ = other.email_;
    nom_ = other.nom_;
    contra_ = other.contra_;
    extra_ = other.extra_;

    id_t_ = other.id_t_;
    desc_t_ = other.desc_t_;
    email_t_ = other.email_t_;
    nom_t_ = other.nom_t_;
    contra_t_ = other.contra_t_;
    extra_t_ = other.extra_t_;
  }
  return *this;
}

// Seguridad
void Cuenta::Encriptar(const string &key) {
  id_ = Seguridad::Encriptar(id_, key, id_t_);
  desc_ = Seguridad::Encriptar(desc_, key, desc_t_);
  email_ = Seguridad::Encriptar(email_, key, email_t_);
  nom_ = Seguridad::Encriptar(nom_, key, nom_t_);
  contra_ = Seguridad::Encriptar(contra_, key, contra_t_);
  extra_ = Seguridad::Encriptar(extra_, key, extra_t_);
}

void Cuenta::Desencriptar(const string &key) {
  id_ = Seguridad::Desencriptar(id_, key, id_t_);
  desc_ = Seguridad::Desencriptar(desc_, key, desc_t_);
  email_ = Seguridad::Desencriptar(email_, key, email_t_);
  nom_ = Seguridad::Desencriptar(nom_, key, nom_t_);
  contra_ = Seguridad::Desencriptar(contra_, key, contra_t_);
  extra_ = Seguridad::Desencriptar(extra_, key, extra_t_);
}

array<DataBlock, Cuenta::kCantAtri> Cuenta::EscribirDataBlocks(const string &key) {
  Encriptar(key);
  array<DataBlock, Cuenta::kCantAtri> cuentas;

  cuentas[0] = DataBlock(id_);
  cuentas[1] = DataBlock(desc_);
  cuentas[2] = DataBlock(email_);
  cuentas[3] = DataBlock(nom_);
  cuentas[4] = DataBlock(contra_);
  cuentas[5] = DataBlock(extra_);

  cuentas[6] = DataBlock(id_t_);
  cuentas[7] = DataBlock(desc_t_);
  cuentas[8] = DataBlock(email_t_);
  cuentas[9] = DataBlock(nom_t_);
  cuentas[10] = DataBlock(contra_t_);
  cuentas[11] = DataBlock(extra_t_);

  return cuentas;
}