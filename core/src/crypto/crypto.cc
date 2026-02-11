#include "crypto/crypto.h"

bool Crypto::Hash256(std::span<const std::byte> buffer, std::byte* hash) {
  return sha3_256_hash(reinterpret_cast<const uint8_t*>(buffer.data()),
                       buffer.size(), reinterpret_cast<uint8_t*>(hash));
}

bool Crypto::Hash512(std::span<const std::byte> buffer, std::byte* hash) {
  return sha3_512_hash(reinterpret_cast<const uint8_t*>(buffer.data()),
                       buffer.size(), reinterpret_cast<uint8_t*>(hash));
}

bool Crypto::Encrypt(const std::byte* key, const std::byte* iv,
                     std::span<const std::byte> aad,
                     std::span<std::byte> buffer, std::byte* tag) {
  return encrypt_aes_gcm_256(reinterpret_cast<const uint8_t*>(key),
                             reinterpret_cast<const uint8_t*>(iv),
                             reinterpret_cast<const uint8_t*>(aad.data()),
                             aad.size(),
                             reinterpret_cast<uint8_t*>(buffer.data()),
                             buffer.size(), reinterpret_cast<uint8_t*>(tag));
}

bool Crypto::Decrypt(const std::byte* key, const std::byte* iv,
                     std::span<const std::byte> aad,
                     std::span<std::byte> buffer, const std::byte* tag) {
  return decrypt_aes_gcm_256(
      reinterpret_cast<const uint8_t*>(key),
      reinterpret_cast<const uint8_t*>(iv),
      reinterpret_cast<const uint8_t*>(aad.data()), aad.size(),
      reinterpret_cast<uint8_t*>(buffer.data()), buffer.size(),
      reinterpret_cast<const uint8_t*>(tag));
}

bool Crypto::Kdf(std::span<const std::byte> password,
                 std::span<const std::byte> salt, std::span<std::byte> kdf) {
  return argon2id_hash(
      reinterpret_cast<const uint8_t*>(password.data()), password.size(),
      reinterpret_cast<const uint8_t*>(salt.data()), salt.size(),
      reinterpret_cast<uint8_t*>(kdf.data()), kdf.size());
}

bool Crypto::SecureClear(std::span<std::byte> buffer) {
  return secure_set_zero(reinterpret_cast<uint8_t*>(buffer.data()),
                         buffer.size());
}
