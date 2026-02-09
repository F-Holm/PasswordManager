#include "seguridad.hh"
#include "tls_adapter.hh"

auto Seguridad::Hash256(const std::string& str) -> std::string {
  return OpenSslAdapter::Hash256(str);
}

auto Seguridad::Hash256_x(const std::string& str,
                          const size_t& len_rta) -> std::string {
  return OpenSslAdapter::Hash256_x(str, len_rta);
}

auto Seguridad::Encriptar(const std::string& str, std::string key,
                          std::string& tag) -> std::string {
  return OpenSslAdapter::Encriptar(str, key, tag);
}

auto Seguridad::Desencriptar(const std::string& str, std::string key,
                             std::string tag) -> std::string {
  return OpenSslAdapter::Desencriptar(str, key, tag);
}
