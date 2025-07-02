#pragma once

#include <array>
#include <string>

#include "db.h"
#include "data_block.h"

class Cuenta {
private:
  std::string id_, desc_, email_, nom_, contra_, extra_;
  std::string id_t_, desc_t_, email_t_, nom_t_, contra_t_, extra_t_;

  void SetIdTag(const std::string &id_t);
  void SetDescripcionTag(const std::string &desc_t);
  void SetEmailTag(const std::string &email_t);
  void SetNombreUsuarioTag(const std::string &nom_t);
  void SetContraTag(const std::string &contra_t);
  void SetExtraTag(const std::string &extra_t);

public:
  const static uint8_t cantAtributos = 12;

  Cuenta(std::array<DataBlock, Cuenta::cantAtributos> datos, const std::string &key);
  Cuenta(std::array<std::string, Cuenta::cantAtributos> datos, const std::string &key);
  explicit Cuenta(std::array<std::string, (Cuenta::cantAtributos / 2) - 1> datos);
  ~Cuenta();
  auto operator=(const Cuenta &other) -> Cuenta &;

  auto id() const -> std::string;
  void SetId(const std::string &id);

  auto desc() const -> std::string;
  void SetDescripcion(const std::string &desc);

  auto GetEmail() const -> std::string;
  void SetEmail(const std::string &email);

  auto GetNombreUsuario() const -> std::string;
  void SetNombreUsuario(const std::string &nom);

  auto GetContra() const -> std::string;
  void SetContra(const std::string &contra);

  auto GetExtra() const -> std::string;
  void SetExtra(const std::string &extra);

  auto GetIdTag() const -> std::string;
  auto GetDescripcionTag() const -> std::string;
  auto GetEmailTag() const -> std::string;
  auto GetNombreUsuarioTag() const -> std::string;
  auto GetContraTag() const -> std::string;
  auto GetExtraTag() const -> std::string;

  auto EscribirDataBlocks(const std::string &key) -> std::array<DataBlock, Cuenta::cantAtributos>;

  void Encriptar(const std::string &key);
  void Desencriptar(const std::string &key);
};