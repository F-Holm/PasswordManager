#pragma once

#include <array>
#include <string>

#include "db.h"

class Cuenta {
private:
  std::string id, descripcion, email, nombreUsuario, contra, extra;

  std::string idTag, descripcionTag, emailTag, nombreUsuarioTag, contraTag,
      extraTag;

  void setIdTag(const std::string &idTag);
  void setDescripcionTag(const std::string &descripcionTag);
  void setEmailTag(const std::string &emailTag);
  void setNombreUsuarioTag(const std::string &nombreUsuarioTag);
  void setContraTag(const std::string &contraTag);
  void setExtraTag(const std::string &extraTag);

public:
  const static uint8_t cantAtributos = 12;

  Cuenta(std::array<DataBlock, Cuenta::cantAtributos> datos,
         const std::string KEY);
  Cuenta(std::array<std::string, Cuenta::cantAtributos> datos,
         const std::string KEY);
  Cuenta(std::array<std::string, Cuenta::cantAtributos / 2 - 1> datos);
  ~Cuenta();

  std::string getId() const;
  void setId(const std::string &id);

  std::string getDescripcion() const;
  void setDescripcion(const std::string &descripcion);

  std::string getEmail() const;
  void setEmail(const std::string &email);

  std::string getNombreUsuario() const;
  void setNombreUsuario(const std::string &nombreUsuario);

  std::string getContra() const;
  void setContra(const std::string &contra);

  std::string getExtra() const;
  void setExtra(const std::string &extra);

  std::string getIdTag() const;
  std::string getDescripcionTag() const;
  std::string getEmailTag() const;
  std::string getNombreUsuarioTag() const;
  std::string getContraTag() const;
  std::string getExtraTag() const;

  std::array<DataBlock, Cuenta::cantAtributos>
  escribirDataBlocks(const std::string KEY);

  void encriptar(const std::string KEY);
  void desencriptar(const std::string KEY);
};