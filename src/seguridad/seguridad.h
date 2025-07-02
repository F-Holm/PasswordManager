#pragma once

#include <string>

namespace Seg {
auto hash256(std::string str) -> std::string;
auto hash256_x(std::string str, const size_t &LEN_RTA) -> std::string;
auto encriptar(std::string str, std::string key, std::string &tag) -> std::string;
auto desencriptar(std::string str, std::string key, std::string tag) -> std::string;
} // namespace Seguridad