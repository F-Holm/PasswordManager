#include "cuenta.h"

#include <string>

#include "seguridad.h"
#include "data_block.h"

using std::array;
using std::string;

// Constructores, destructores y operators
Cuenta::Cuenta(array<DataBlock, Cuenta::cantAtributos> datos, const string &key) {
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

Cuenta::Cuenta(array<string, Cuenta::cantAtributos> datos, const string &key) {
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

Cuenta::Cuenta(array<string, (Cuenta::cantAtributos / 2) - 1> datos) {
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

auto Cuenta::operator=(const Cuenta &other) -> Cuenta & {}

// Getters y Setters
string Cuenta::GetId() const { return id_; }
void Cuenta::SetId(const string &id) { id_ = id; }

string Cuenta::GetDescripcion() const { return desc_; }
void Cuenta::SetDescripcion(const string &desc) {
  desc_ = desc;
}

string Cuenta::GetEmail() const { return email_; }
void Cuenta::SetEmail(const string &email) { email_ = email; }

string Cuenta::GetNombreUsuario() const { return nom_; }
void Cuenta::SetNombreUsuario(const string &nom) {
  nom_ = nom;
}

string Cuenta::GetContra() const { return contra_; }
void Cuenta::SetContra(const string &contra) { contra_ = contra; }

string Cuenta::GetExtra() const { return extra_; }
void Cuenta::SetExtra(const string &extra) { extra_ = extra; }

string Cuenta::GetIdTag() const { return id_t_; }
string Cuenta::GetDescripcionTag() const { return desc_t_; }
string Cuenta::GetEmailTag() const { return email_t_; }
string Cuenta::GetNombreUsuarioTag() const { return nom_t_; }
string Cuenta::GetContraTag() const { return contra_t_; }
string Cuenta::GetExtraTag() const { return extra_t_; }

void Cuenta::SetIdTag(const string &id_t) { id_t_ = id_t; }
void Cuenta::SetDescripcionTag(const string &desc_t) {
  desc_t_ = desc_t;
}
void Cuenta::SetEmailTag(const string &email_t) { email_t_ = email_t; }
void Cuenta::SetNombreUsuarioTag(const string &nom_t) {
  nom_t_ = nom_t;
}
void Cuenta::SetContraTag(const string &contra_t) {
  contra_t_ = contra_t;
}
void Cuenta::SetExtraTag(const string &extra_t) { extra_t_ = extra_t; }

// Seguridad
void Cuenta::Encriptar(const string &key) {
  id_ = Seg::Encriptar(id_, key, id_t_);
  desc_ = Seg::Encriptar(desc_, key, desc_t_);
  email_ = Seg::Encriptar(email_, key, email_t_);
  nom_ = Seg::Encriptar(nom_, key, nom_t_);
  contra_ = Seg::Encriptar(contra_, key, contra_t_);
  extra_ = Seg::Encriptar(extra_, key, extra_t_);
}
void Cuenta::Desencriptar(const string &key) {
  id_ = Seg::desencriptar(id_, key, id_t_);
  desc_ = Seg::desencriptar(desc_, key, desc_t_);
  email_ = Seg::desencriptar(email_, key, email_t_);
  nom_ = Seg::desencriptar(nom_, key, nom_t_);
  contra_ = Seg::desencriptar(contra_, key, contra_t_);
  extra_ = Seg::desencriptar(extra_, key, extra_t_);
}

array<DataBlock, Cuenta::cantAtributos> Cuenta::EscribirDataBlocks(const string &key) {
  Encriptar(key);
  array<DataBlock, Cuenta::cantAtributos> cuentas;

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