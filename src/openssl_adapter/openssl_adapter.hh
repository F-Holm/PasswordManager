#pragma once

#include <expected>
#include <string>

namespace OpenSslAdapter {
auto Hash256(const std::string& str) -> std::string;
auto Hash256_x(const std::string& str, const size_t& len_rta) -> std::string;
auto Encriptar(const std::string& str, std::string key,
               std::string& tag) -> std::string;
auto Desencriptar(const std::string& str, std::string key,
                  std::string tag) -> std::string;
}  // namespace OpenSslAdapter