#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "data_block.hh"
#include "db.hh"

class Cuenta {
 public:
  inline static constexpr uint8_t kCantAtributos = 12;

  Cuenta(std::array<DataBlock, Cuenta::kCantAtributos> datos,
         const std::string &key);
  Cuenta(std::array<std::string, Cuenta::kCantAtributos> datos,
         const std::string &key);
  explicit Cuenta(
      std::array<std::string, Cuenta::kCantAtributos / 2> datos);  // Con id
  explicit Cuenta(std::array<std::string, (Cuenta::kCantAtributos / 2) - 1>
                      datos);  // Sin id
  ~Cuenta();
  auto operator=(const Cuenta &other) -> Cuenta &;

  // Setters
  void SetId(const std::string &id) { id_ = id; };
  void SetDescripcion(const std::string &descripcion) {
    descripcion_ = descripcion;
  };
  void SetEmail(const std::string &email) { email_ = email; };
  void SetNombreUsuario(const std::string &nombre_usuario) {
    nombre_usuario_ = nombre_usuario;
  };
  void SetContra(const std::string &contra) { contra_ = contra; };
  void SetExtra(const std::string &extra) { extra_ = extra; };

  // Getters
  auto id() const -> std::string { return id_; };
  auto descripcion() const -> std::string { return descripcion_; };
  auto email() const -> std::string { return email_; };
  auto nombre_usuario() const -> std::string { return nombre_usuario_; };
  auto contra() const -> std::string { return contra_; };
  auto extra() const -> std::string { return extra_; };

  // Getters Tags
  auto id_tag() const -> std::string { return id_tag_; };
  auto descripcion_tag() const -> std::string { return descripcion_tag_; };
  auto email_tag() const -> std::string { return email_tag_; };
  auto nombre_usuario_tag() const -> std::string {
    return nombre_usuario_tag_;
  };
  auto contra_tag() const -> std::string { return contra_tag_; };
  auto extra_tag() const -> std::string { return extra_tag_; };

  auto EscribirDataBlocks(const std::string &key)
      -> std::array<DataBlock, Cuenta::kCantAtributos>;

  void Encriptar(const std::string &key);
  void Desencriptar(const std::string &key);

 private:
  std::string id_, descripcion_, email_, nombre_usuario_, contra_, extra_;
  std::string id_tag_, descripcion_tag_, email_tag_, nombre_usuario_tag_,
      contra_tag_, extra_tag_;

  // Setters Tags
  void SetIdTag(const std::string &id_tag) { id_tag_ = id_tag; };
  void SetDescripcionTag(const std::string &descripcion_tag) {
    descripcion_tag_ = descripcion_tag;
  };
  void SetEmailTag(const std::string &email_tag) { email_tag_ = email_tag; };
  void SetNombreUsuarioTag(const std::string &nombre_usuario_tag) {
    nombre_usuario_tag_ = nombre_usuario_tag;
  };
  void SetContraTag(const std::string &contra_tag) {
    contra_tag_ = contra_tag;
  };
  void SetExtraTag(const std::string &extra_tag) { extra_tag_ = extra_tag; };
};