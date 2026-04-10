#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <ostream>

constexpr static const size_t SHA3_256_OUT_SIZE = 32;

constexpr static const size_t SHA3_512_OUT_SIZE = 64;

constexpr static const size_t AES256_KEY_SIZE = 32;

constexpr static const size_t AES256_IV_SIZE = 12;

constexpr static const size_t AES256_TAG_SIZE = 16;

extern "C" {

bool sha3_256_hash(const uint8_t *input, size_t len, uint8_t *output);

bool sha3_512_hash(const uint8_t *input, size_t len, uint8_t *output);

bool encrypt_aes_gcm_256(const uint8_t *key, const uint8_t *iv,
                         const uint8_t *aad, size_t aad_len, uint8_t *data,
                         size_t data_len, uint8_t *tag_out);

bool decrypt_aes_gcm_256(const uint8_t *key, const uint8_t *iv,
                         const uint8_t *aad, size_t aad_len, uint8_t *data,
                         size_t data_len, const uint8_t *tag);

bool argon2id_hash(const uint8_t *password, size_t pass_len,
                   const uint8_t *salt, size_t salt_len, uint8_t *output,
                   size_t output_len);

bool secure_set_zero(uint8_t *ptr, size_t len);

}  // extern "C"
