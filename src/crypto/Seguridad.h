#pragma once

#include <string>

namespace Seguridad {
std::string hash256(std::string str);
std::string hash256_x(std::string str, const size_t &LEN_RTA);
std::string encriptar(std::string str, std::string key, std::string &tag);
std::string desencriptar(std::string str, std::string key, std::string tag);
} // namespace Seguridad