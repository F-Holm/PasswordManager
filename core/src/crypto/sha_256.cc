#include "crypto/sha_256.h"

#include <openssl/sha.h>

#include <cstring>

void Sha256::Sha256(std::string_view str, char* rta) noexcept {
  SHA256(reinterpret_cast<const unsigned char*>(str.data()), str.size(),
         reinterpret_cast<unsigned char*>(rta));
}

void Sha256::Sha256_X(std::string_view str, char* rta,
                      size_t len_rta) noexcept {
  char temp_rta[SHA256_DIGEST_LENGTH];
  Sha256::Hash256(str, temp_rta);
  memcpy(rta, temp_rta,
         len_rta <= SHA256_DIGEST_LENGTH ? len_rta : SHA256_DIGEST_LENGTH);
}
