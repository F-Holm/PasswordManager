#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "rust_crypto/rust_crypto.h"

namespace Crypto {
namespace Size {
constexpr std::size_t kSha256 = SHA3_256_OUT_SIZE;
constexpr std::size_t kSha512 = SHA3_512_OUT_SIZE;
constexpr std::size_t kKey = AES256_KEY_SIZE;
constexpr std::size_t kIv = AES256_IV_SIZE;
constexpr std::size_t kTag = AES256_TAG_SIZE;
constexpr std::size_t kSalt = 16;
}  // namespace Size

// Argon2id cost parameters. Stored alongside each vault so they can be
// raised for new vaults without breaking existing ones.
struct KdfParams {
  std::uint32_t memory_kib;
  std::uint32_t iterations;
  std::uint32_t parallelism;

  friend bool operator==(const KdfParams&, const KdfParams&) = default;
};

// Defaults for new vaults: 64 MiB, 3 passes, 4 lanes.
constexpr KdfParams kDefaultKdfParams{65536, 3, 4};

bool Hash256(std::span<const std::byte> buffer, std::byte* hash);
bool Hash512(std::span<const std::byte> buffer, std::byte* hash);
bool Encrypt(const std::byte* key, const std::byte* iv,
             std::span<const std::byte> aad, std::span<std::byte> buffer,
             std::byte* tag);
bool Decrypt(const std::byte* key, const std::byte* iv,
             std::span<const std::byte> aad, std::span<std::byte> buffer,
             const std::byte* tag);
bool Kdf(std::span<const std::byte> password, std::span<const std::byte> salt,
         std::span<std::byte> kdf);
// Argon2id with explicit cost parameters. Returns false if they are invalid
// (memory below 8 KiB per lane, zero iterations or lanes, salt shorter than
// 8 bytes).
bool Kdf(std::span<const std::byte> password, std::span<const std::byte> salt,
         const KdfParams& params, std::span<std::byte> kdf);
bool SecureClear(std::span<std::byte> buffer);
// Fills `buffer` with bytes from the operating system's cryptographically
// secure random generator. Returns false (with `buffer` zeroed) on failure.
bool RandomBytes(std::span<std::byte> buffer);
}  // namespace Crypto
