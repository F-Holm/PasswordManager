#include "Cuenta.h"

#include "Seguridad.h"

using std::array;
using std::string;

Cuenta::Cuenta(array<DataBlock, Cuenta::cantAtributos> datos,
               const string KEY) {
  this->id =
      string(reinterpret_cast<const char *>(datos[0].str), datos[0].largo);
  this->descripcion =
      string(reinterpret_cast<const char *>(datos[1].str), datos[1].largo);
  this->email =
      string(reinterpret_cast<const char *>(datos[2].str), datos[2].largo);
  this->nombreUsuario =
      string(reinterpret_cast<const char *>(datos[3].str), datos[3].largo);
  this->contra =
      string(reinterpret_cast<const char *>(datos[4].str), datos[4].largo);
  this->extra =
      string(reinterpret_cast<const char *>(datos[5].str), datos[5].largo);

  this->idTag =
      string(reinterpret_cast<const char *>(datos[6].str), datos[6].largo);
  this->descripcionTag =
      string(reinterpret_cast<const char *>(datos[7].str), datos[7].largo);
  this->emailTag =
      string(reinterpret_cast<const char *>(datos[8].str), datos[8].largo);
  this->nombreUsuarioTag =
      string(reinterpret_cast<const char *>(datos[9].str), datos[9].largo);
  this->contraTag =
      string(reinterpret_cast<const char *>(datos[10].str), datos[10].largo);
  this->extraTag =
      string(reinterpret_cast<const char *>(datos[11].str), datos[11].largo);

  desencriptar(KEY);
}
Cuenta::Cuenta(array<string, Cuenta::cantAtributos> datos, const string KEY) {
  this->id = datos[0];
  this->descripcion = datos[1];
  this->email = datos[2];
  this->nombreUsuario = datos[3];
  this->contra = datos[4];
  this->extra = datos[5];

  this->idTag = datos[6];
  this->descripcionTag = datos[7];
  this->emailTag = datos[8];
  this->nombreUsuarioTag = datos[9];
  this->contraTag = datos[10];
  this->extraTag = datos[11];

  desencriptar(KEY);
}
Cuenta::Cuenta(array<string, Cuenta::cantAtributos / 2 - 1> datos) {
  this->id = "";
  this->descripcion = datos[0];
  this->email = datos[1];
  this->nombreUsuario = datos[2];
  this->contra = datos[3];
  this->extra = datos[4];

  this->idTag = "";
  this->descripcionTag = "";
  this->emailTag = "";
  this->nombreUsuarioTag = "";
  this->contraTag = "";
  this->extraTag = "";
}
Cuenta::~Cuenta() {}

string Cuenta::getId() const { return this->id; }
void Cuenta::setId(const string &id) { this->id = id; }

string Cuenta::getDescripcion() const { return this->descripcion; }
void Cuenta::setDescripcion(const string &descripcion) {
  this->descripcion = descripcion;
}

string Cuenta::getEmail() const { return this->email; }
void Cuenta::setEmail(const string &email) { this->email = email; }

string Cuenta::getNombreUsuario() const { return this->nombreUsuario; }
void Cuenta::setNombreUsuario(const string &nombreUsuario) {
  this->nombreUsuario = nombreUsuario;
}

string Cuenta::getContra() const { return this->contra; }
void Cuenta::setContra(const string &contra) { this->contra = contra; }

string Cuenta::getExtra() const { return this->extra; }
void Cuenta::setExtra(const string &extra) { this->extra = extra; }

string Cuenta::getIdTag() const { return this->idTag; }
string Cuenta::getDescripcionTag() const { return this->descripcionTag; }
string Cuenta::getEmailTag() const { return this->emailTag; }
string Cuenta::getNombreUsuarioTag() const { return this->nombreUsuarioTag; }
string Cuenta::getContraTag() const { return this->contraTag; }
string Cuenta::getExtraTag() const { return this->extraTag; }

void Cuenta::setIdTag(const string &idTag) { this->idTag = idTag; }
void Cuenta::setDescripcionTag(const string &descripcionTag) {
  this->descripcionTag = descripcionTag;
}
void Cuenta::setEmailTag(const string &emailTag) { this->emailTag = emailTag; }
void Cuenta::setNombreUsuarioTag(const string &nombreUsuarioTag) {
  this->nombreUsuarioTag = nombreUsuarioTag;
}
void Cuenta::setContraTag(const string &contraTag) {
  this->contraTag = contraTag;
}
void Cuenta::setExtraTag(const string &extraTag) { this->extraTag = extraTag; }

void Cuenta::encriptar(const string KEY) {
  id = Seguridad::encriptar(id, KEY, idTag);
  this->descripcion =
      Seguridad::encriptar(this->descripcion, KEY, this->descripcionTag);
  email = Seguridad::encriptar(email, KEY, emailTag);
  nombreUsuario = Seguridad::encriptar(nombreUsuario, KEY, nombreUsuarioTag);
  contra = Seguridad::encriptar(contra, KEY, contraTag);
  extra = Seguridad::encriptar(extra, KEY, extraTag);
}
void Cuenta::desencriptar(const string KEY) {
  id = Seguridad::desencriptar(id, KEY, idTag);
  this->descripcion =
      Seguridad::desencriptar(this->descripcion, KEY, this->descripcionTag);
  email = Seguridad::desencriptar(email, KEY, emailTag);
  nombreUsuario = Seguridad::desencriptar(nombreUsuario, KEY, nombreUsuarioTag);
  contra = Seguridad::desencriptar(contra, KEY, contraTag);
  extra = Seguridad::desencriptar(extra, KEY, extraTag);
}

array<DataBlock, Cuenta::cantAtributos>
Cuenta::escribirDataBlocks(const string KEY) {
  encriptar(KEY);
  array<DataBlock, Cuenta::cantAtributos> cuentas;

  cuentas[0] = DataBlock(id);
  cuentas[1] = DataBlock(descripcion);
  cuentas[2] = DataBlock(email);
  cuentas[3] = DataBlock(nombreUsuario);
  cuentas[4] = DataBlock(contra);
  cuentas[5] = DataBlock(extra);

  cuentas[6] = DataBlock(idTag);
  cuentas[7] = DataBlock(descripcionTag);
  cuentas[8] = DataBlock(emailTag);
  cuentas[9] = DataBlock(nombreUsuarioTag);
  cuentas[10] = DataBlock(contraTag);
  cuentas[11] = DataBlock(extraTag);

  return cuentas;
}