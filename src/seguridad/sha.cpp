#include "seguridad.h"

#include <openssl/sha.h>

using std::string;

auto Seguridad::hash256(string str) -> string {
  unsigned char rta[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<unsigned char *>(&str[0]), str.size(), rta);
  return string(reinterpret_cast<const char *>(rta), SHA256_DIGEST_LENGTH);
}

auto Seguridad::hash256_x(string str, const size_t &LEN_RTA) -> string {
  return Seguridad::hash256(str).substr(0, LEN_RTA);
}