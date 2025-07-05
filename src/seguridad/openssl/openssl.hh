#pragma once

#include <string>

namespace OpenSSL_ {
inline auto hash256(std::string str) -> std::string;
inline auto hash256_x(std::string str, const size_t &LEN_RTA) -> std::string;
inline auto encriptar(std::string str, std::string key, std::string &tag) -> std::string;
inline auto desencriptar(std::string str, std::string key, std::string tag) -> std::string;
} // namespace Seguridad