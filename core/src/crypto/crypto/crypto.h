#pragma once

#include <cstdint>
#include <span>
#include <cstddef>

#include "rust_crypto/rust_crypto.h"

namespace Crypto {
    namespace Size {
        constexpr std::size_t kSha256 = SHA3_256_OUT_SIZE;
        constexpr std::size_t kSha512 = SHA3_512_OUT_SIZE;
        constexpr std::size_t kKey = AES256_KEY_SIZE;
        constexpr std::size_t kIv = AES256_IV_SIZE;
        constexpr std::size_t kTag = AES256_TAG_SIZE;
    } // Size

    bool Hash256(std::span<const std::byte> buffer, std::byte* hash);
    bool Hash512(std::span<const std::byte> buffer, std::byte* hash);
    bool Encrypt(const std::byte* key, const std::byte* iv, std::span<const std::byte> aad, std::span<std::byte> buffer, std::byte* tag);
    bool Decrypt(const std::byte* key, const std::byte* iv, std::span<const std::byte> aad, std::span<std::byte> buffer, const std::byte* tag);
    bool Kdf(std::span<const std::byte> password, std::span<const std::byte> salt, std::span<std::byte> kdf);
    bool SecureClear(std::span<std::byte> buffer);
} // Crypto
