#pragma once

#include <string>

namespace Seg {
auto Hash256(std::string str) -> std::string;
auto Hash256_x(std::string str, const size_t &len_rta) -> std::string;
auto Encriptar(std::string str, std::string key, std::string &tag) -> std::string;
auto Desencriptar(std::string str, std::string key, std::string tag) -> std::string;
} // namespace Seguridad