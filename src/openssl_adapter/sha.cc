#include "openssl_adapter.hh"

#include <openssl/sha.h>

auto OpenSslAdapter::Hash256(const std::string& str) -> std::string {
  unsigned char rta[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const unsigned char *>(str.c_str()), str.size(), rta);
  return { reinterpret_cast<const char *>(rta), SHA256_DIGEST_LENGTH };
}

auto OpenSslAdapter::Hash256_x(const std::string& str, const size_t& len_rta) -> std::string {
  return OpenSslAdapter::Hash256(str).substr(0, len_rta);
}