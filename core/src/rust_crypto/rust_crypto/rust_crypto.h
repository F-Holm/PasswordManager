#pragma once

#include <cstdint>
#include <cstddef>

const size_t SHA3_256_OUT_SIZE = 32;
const size_t SHA3_512_OUT_SIZE = 64;
const size_t AES256_KEY_SIZE = 32;
const size_t AES256_IV_SIZE = 12;
const size_t AES256_TAG_SIZE = 16;
const size_t ARGON2_OUT_SIZE = 32;

extern "C" {

    bool sha3_256_hash(const uint8_t* input, uint64_t len, uint8_t* output);

    bool sha3_512_hash(const uint8_t* input, uint64_t len, uint8_t* output);

    bool encrypt_aes_gcm_256(
        const uint8_t* key, 
        const uint8_t* iv, 
        const uint8_t* aad, 
        uint64_t aad_len,
        uint8_t* data, 
        uint64_t data_len, 
        uint8_t* tag_out
    );

    bool decrypt_aes_gcm_256(
        const uint8_t* key, 
        const uint8_t* iv, 
        const uint8_t* aad, 
        uint64_t aad_len,
        const uint8_t* tag, 
        uint8_t* data, 
        uint64_t data_len
    );

    bool argon2id_hash(
        const uint8_t* password, 
        uint64_t pass_len, 
        const uint8_t* salt, 
        uint64_t salt_len, 
        uint8_t* output
    );

    bool secure_set_zero(uint8_t* ptr, uint64_t len);

} // extern "C"
