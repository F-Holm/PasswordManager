#include "openssl.hh"

#include <openssl/sha.h>

using std::string;

inline auto OpenSSL_::hash256(string str) -> string {
  char rta[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<unsigned char *>(&str[0]), str.size(), rta);
  return string(reinterpret_cast<const char *>(rta), SHA256_DIGEST_LENGTH);
}

inline auto OpenSSL_::hash256_x(string str, const size_t &len_rta) -> string {
  return OpenSSL_::hash256(str).substr(0, len_rta);
}