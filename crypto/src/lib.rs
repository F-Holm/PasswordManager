use std::slice;
use sha3::{Sha3_256, Sha3_512, Digest};
use aes_gcm::{Aes256Gcm, Key, Nonce, KeyInit, Tag};
use aes_gcm::aead::AeadInPlace;
use argon2::{Argon2, PasswordHasher};
use zeroize::Zeroize;

// Constants to avoid magic numbers and sync with C++
pub const SHA3_256_OUT_SIZE: usize = 32;
pub const SHA3_512_OUT_SIZE: usize = 64;
pub const AES256_KEY_SIZE: usize = 32;
pub const AES256_IV_SIZE: usize = 12;
pub const AES256_TAG_SIZE: usize = 16;
pub const ARGON2_OUT_SIZE: usize = 32;

/// Computes SHA3-256 hash. Output buffer must be 32 bytes.
#[no_mangle]
pub extern "C" fn sha3_256_hash(input: *const u8, len: u64, output: *mut u8) {
    let input_slice = unsafe { slice::from_raw_parts(input, len as usize) };
    let output_slice = unsafe { slice::from_raw_parts_mut(output, SHA3_256_OUT_SIZE) };
    
    let mut hasher = Sha3_256::new();
    hasher.update(input_slice);
    output_slice.copy_from_slice(&hasher.finalize());
}

/// Computes SHA3-512 hash. Output buffer must be 64 bytes.
#[no_mangle]
pub extern "C" fn sha3_512_hash(input: *const u8, len: u64, output: *mut u8) {
    let input_slice = unsafe { slice::from_raw_parts(input, len as usize) };
    let output_slice = unsafe { slice::from_raw_parts_mut(output, SHA3_512_OUT_SIZE) };
    
    let mut hasher = Sha3_512::new();
    hasher.update(input_slice);
    output_slice.copy_from_slice(&hasher.finalize());
}

/// Encrypts data in-place using AES-256-GCM.
#[no_mangle]
pub extern "C" fn encrypt_aes_gcm_256(
    key: *const u8,         // 32 bytes
    iv: *const u8,          // 12 bytes
    aad: *const u8,
    aad_len: u64,
    data: *mut u8,          // In-place buffer
    data_len: u64,
    tag_out: *mut u8        // 16 bytes
) -> bool {
    let key_slice = unsafe { slice::from_raw_parts(key, AES256_KEY_SIZE) };
    let iv_slice = unsafe { slice::from_raw_parts(iv, AES256_IV_SIZE) };
    let aad_slice = unsafe { slice::from_raw_parts(aad, aad_len as usize) };
    let data_slice = unsafe { slice::from_raw_parts_mut(data, data_len as usize) };
    
    let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(key_slice));
    let nonce = Nonce::from_slice(iv_slice);

    match cipher.encrypt_in_place_detached(nonce, aad_slice, data_slice) {
        Ok(tag) => {
            let out_tag = unsafe { slice::from_raw_parts_mut(tag_out, AES256_TAG_SIZE) };
            out_tag.copy_from_slice(&tag);
            true
        },
        Err(_) => false,
    }
}

/// Decrypts data in-place using AES-256-GCM.
#[no_mangle]
pub extern "C" fn decrypt_aes_gcm_256(
    key: *const u8,
    iv: *const u8,
    aad: *const u8,
    aad_len: u64,
    tag: *const u8,         // 16 bytes
    data: *mut u8,          // In-place buffer
    data_len: u64
) -> bool {
    let key_slice = unsafe { slice::from_raw_parts(key, AES256_KEY_SIZE) };
    let iv_slice = unsafe { slice::from_raw_parts(iv, AES256_IV_SIZE) };
    let aad_slice = unsafe { slice::from_raw_parts(aad, aad_len as usize) };
    let tag_slice = unsafe { slice::from_raw_parts(tag, AES256_TAG_SIZE) };
    let data_slice = unsafe { slice::from_raw_parts_mut(data, data_len as usize) };
    
    let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(key_slice));
    let nonce = Nonce::from_slice(iv_slice);
    let gcm_tag = Tag::from_slice(tag_slice);

    cipher.decrypt_in_place_detached(nonce, aad_slice, data_slice, gcm_tag).is_ok()
}

/// Hashes password using Argon2id.
#[no_mangle]
pub extern "C" fn argon2id_hash(
    password: *const u8,
    pass_len: u64,
    salt: *const u8,
    salt_len: u64,
    output: *mut u8        // 32 bytes
) -> bool {
    let pwd = unsafe { slice::from_raw_parts(password, pass_len as usize) };
    let slt = unsafe { slice::from_raw_parts(salt, salt_len as usize) };
    let out = unsafe { slice::from_raw_parts_mut(output, ARGON2_OUT_SIZE) };

    let argon2 = Argon2::default();
    argon2.hash_password_into(pwd, slt, out).is_ok()
}

/// Securely clears memory to prevent leaking sensitive data.
#[no_mangle]
pub extern "C" fn secure_set_zero(ptr: *mut u8, len: u64) {
    let data = unsafe { slice::from_raw_parts_mut(ptr, len as usize) };
    data.zeroize();
}
