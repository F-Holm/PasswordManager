#pragma once

#include <array>
#include <string>

#include "db.hh"
#include "data_block.hh"

class Cuenta {
public:
  const static uint8_t cantAtributos = 12;

  Cuenta(std::array<DataBlock, Cuenta::cantAtributos> datos, const std::string &key);
  Cuenta(std::array<std::string, Cuenta::cantAtributos> datos, const std::string &key);
  explicit Cuenta(std::array<std::string, (Cuenta::cantAtributos / 2) - 1> datos);
  ~Cuenta();
  auto operator=(const Cuenta &other) -> Cuenta &;

  // Setters
  void SetId(const std::string &id) { id_ = id; };
  void SetDescripcion(const std::string &desc) { desc_ = desc; };
  void SetEmail(const std::string &email) { email_ = email; };
  void SetNombreUsuario(const std::string &nom) { nom_ = nom; };
  void SetContra(const std::string &contra) { contra_ = contra; };
  void SetExtra(const std::string &extra) { extra_ = extra; };

  //Getters
  auto id() const -> std::string { return id_; };
  auto desc() const -> std::string { return desc_; };
  auto email() const -> std::string { return email_; };
  auto nom() const -> std::string { return nom_; };
  auto contra() const -> std::string { return contra_; };
  auto extra() const -> std::string { return extra_; };

  // Getters Tags
  auto id_t() const -> std::string { return id_t_; };
  auto desc_t() const -> std::string { return desc_t_; };
  auto email_t() const -> std::string { return email_t_; };
  auto nom_t() const -> std::string { return nom_t_; };
  auto contra_t() const -> std::string { return contra_t_; };
  auto extra_t() const -> std::string { return extra_t_; };

  auto EscribirDataBlocks(const std::string &key) -> std::array<DataBlock, Cuenta::cantAtributos>;

  void Encriptar(const std::string &key);
  void Desencriptar(const std::string &key);

  private:
  std::string id_, desc_, email_, nom_, contra_, extra_;
  std::string id_t_, desc_t_, email_t_, nom_t_, contra_t_, extra_t_;

  // Setters Tags
  void SetIdTag(const std::string &id_t) { id_t_ = id_t; };
  void SetDescripcionTag(const std::string &desc_t) { desc_t_ = desc_t; };
  void SetEmailTag(const std::string &email_t) { email_t_ = email_t; };
  void SetNombreUsuarioTag(const std::string &nom_t) { nom_t_ = nom_t; };
  void SetContraTag(const std::string &contra_t) { contra_t_ = contra_t; };
  void SetExtraTag(const std::string &extra_t) { extra_t_ = extra_t; };
};