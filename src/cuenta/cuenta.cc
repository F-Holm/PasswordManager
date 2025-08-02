#include "cuenta.hh"

#include "data_block.hh"
#include "seguridad.hh"

// Constructores, destructores y operators
Cuenta::Cuenta(std::array<DataBlock, Cuenta::kCantAtributos> datos,
               const std::string &key) {
  id_ =
      std::string(reinterpret_cast<const char *>(datos[0].str), datos[0].largo);
  descripcion_ =
      std::string(reinterpret_cast<const char *>(datos[1].str), datos[1].largo);
  email_ =
      std::string(reinterpret_cast<const char *>(datos[2].str), datos[2].largo);
  nombre_usuario_ =
      std::string(reinterpret_cast<const char *>(datos[3].str), datos[3].largo);
  contra_ =
      std::string(reinterpret_cast<const char *>(datos[4].str), datos[4].largo);
  extra_ =
      std::string(reinterpret_cast<const char *>(datos[5].str), datos[5].largo);

  id_tag_ =
      std::string(reinterpret_cast<const char *>(datos[6].str), datos[6].largo);
  descripcion_tag_ =
      std::string(reinterpret_cast<const char *>(datos[7].str), datos[7].largo);
  email_tag_ =
      std::string(reinterpret_cast<const char *>(datos[8].str), datos[8].largo);
  nombre_usuario_tag_ =
      std::string(reinterpret_cast<const char *>(datos[9].str), datos[9].largo);
  contra_tag_ = std::string(reinterpret_cast<const char *>(datos[10].str),
                            datos[10].largo);
  extra_tag_ = std::string(reinterpret_cast<const char *>(datos[11].str),
                           datos[11].largo);

  Desencriptar(key);
}

Cuenta::Cuenta(std::array<std::string, Cuenta::kCantAtributos> datos,
               const std::string &key) {
  id_ = datos[0];
  descripcion_ = datos[1];
  email_ = datos[2];
  nombre_usuario_ = datos[3];
  contra_ = datos[4];
  extra_ = datos[5];

  id_tag_ = datos[6];
  descripcion_tag_ = datos[7];
  email_tag_ = datos[8];
  nombre_usuario_tag_ = datos[9];
  contra_tag_ = datos[10];
  extra_tag_ = datos[11];

  Desencriptar(key);
}

Cuenta::Cuenta(std::array<std::string, Cuenta::kCantAtributos / 2> datos) {
  id_ = datos[0];
  descripcion_ = datos[1];
  email_ = datos[2];
  nombre_usuario_ = datos[3];
  contra_ = datos[4];
  extra_ = datos[5];

  id_tag_ = "";
  descripcion_tag_ = "";
  email_tag_ = "";
  nombre_usuario_tag_ = "";
  contra_tag_ = "";
  extra_tag_ = "";
}

Cuenta::Cuenta(
    std::array<std::string, (Cuenta::kCantAtributos / 2) - 1> datos) {
  id_ = "";
  descripcion_ = datos[0];
  email_ = datos[1];
  nombre_usuario_ = datos[2];
  contra_ = datos[3];
  extra_ = datos[4];

  id_tag_ = "";
  descripcion_tag_ = "";
  email_tag_ = "";
  nombre_usuario_tag_ = "";
  contra_tag_ = "";
  extra_tag_ = "";
}

Cuenta::~Cuenta() {}

auto Cuenta::operator=(const Cuenta &other) -> Cuenta & {
  if (this != &other) {
    id_ = other.id_;
    descripcion_ = other.descripcion_;
    email_ = other.email_;
    nombre_usuario_ = other.nombre_usuario_;
    contra_ = other.contra_;
    extra_ = other.extra_;

    id_tag_ = other.id_tag_;
    descripcion_tag_ = other.descripcion_tag_;
    email_tag_ = other.email_tag_;
    nombre_usuario_tag_ = other.nombre_usuario_tag_;
    contra_tag_ = other.contra_tag_;
    extra_tag_ = other.extra_tag_;
  }
  return *this;
}

// Seguridad
void Cuenta::Encriptar(const std::string &key) {
  id_ = Seguridad::Encriptar(id_, key, id_tag_);
  descripcion_ = Seguridad::Encriptar(descripcion_, key, descripcion_tag_);
  email_ = Seguridad::Encriptar(email_, key, email_tag_);
  nombre_usuario_ =
      Seguridad::Encriptar(nombre_usuario_, key, nombre_usuario_tag_);
  contra_ = Seguridad::Encriptar(contra_, key, contra_tag_);
  extra_ = Seguridad::Encriptar(extra_, key, extra_tag_);
}

void Cuenta::Desencriptar(const std::string &key) {
  id_ = Seguridad::Desencriptar(id_, key, id_tag_);
  descripcion_ = Seguridad::Desencriptar(descripcion_, key, descripcion_tag_);
  email_ = Seguridad::Desencriptar(email_, key, email_tag_);
  nombre_usuario_ =
      Seguridad::Desencriptar(nombre_usuario_, key, nombre_usuario_tag_);
  contra_ = Seguridad::Desencriptar(contra_, key, contra_tag_);
  extra_ = Seguridad::Desencriptar(extra_, key, extra_tag_);
}

std::array<DataBlock, Cuenta::kCantAtributos> Cuenta::EscribirDataBlocks(
    const std::string &key) {
  Encriptar(key);
  std::array<DataBlock, Cuenta::kCantAtributos> cuentas;

  cuentas[0] = DataBlock(id_);
  cuentas[1] = DataBlock(descripcion_);
  cuentas[2] = DataBlock(email_);
  cuentas[3] = DataBlock(nombre_usuario_);
  cuentas[4] = DataBlock(contra_);
  cuentas[5] = DataBlock(extra_);

  cuentas[6] = DataBlock(id_tag_);
  cuentas[7] = DataBlock(descripcion_tag_);
  cuentas[8] = DataBlock(email_tag_);
  cuentas[9] = DataBlock(nombre_usuario_tag_);
  cuentas[10] = DataBlock(contra_tag_);
  cuentas[11] = DataBlock(extra_tag_);

  return cuentas;
}