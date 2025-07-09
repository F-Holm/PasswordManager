#include "openssl_.hh"

#include <openssl/sha.h>

using std::string;

auto OpenSSL_::Hash256(string str) -> string {
  char rta[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<unsigned char *>(&str[0]), str.size(), rta);
  return string(reinterpret_cast<const char *>(rta), SHA256_DIGEST_LENGTH);
}

auto OpenSSL_::Hash256_x(string str, const size_t &len_rta) -> string {
  return OpenSSL_::Hash256(str).substr(0, len_rta);
}