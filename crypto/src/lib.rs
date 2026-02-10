use aes_gcm::aead::AeadInPlace;
use aes_gcm::{Aes256Gcm, Key, KeyInit, Nonce, Tag};
use argon2::Argon2;
use sha3::{Digest, Sha3_256, Sha3_512};
use std::slice;
use zeroize::Zeroize;

// --- Constants ---
pub const SHA3_256_OUT_SIZE: usize = 32;
pub const SHA3_512_OUT_SIZE: usize = 64;
pub const AES256_KEY_SIZE: usize = 32;
pub const AES256_IV_SIZE: usize = 12;
pub const AES256_TAG_SIZE: usize = 16;
pub const ARGON2_OUT_SIZE: usize = 32;

/// Macro to verify that pointers are not null before proceeding
macro_rules! check_null {
    ($($ptr:expr),*) => {
        $( if $ptr.is_null() { return false; } )*
    };
}

// --- FFI Functions ---

#[no_mangle]
pub extern "C" fn sha3_256_hash(input: *const u8, len: u64, output: *mut u8) -> bool {
    check_null!(input, output);
    let input_slice = unsafe { slice::from_raw_parts(input, len as usize) };
    let output_slice = unsafe { slice::from_raw_parts_mut(output, SHA3_256_OUT_SIZE) };

    let mut hasher = Sha3_256::new();
    hasher.update(input_slice);
    output_slice.copy_from_slice(&hasher.finalize());
    true
}

#[no_mangle]
pub extern "C" fn sha3_512_hash(input: *const u8, len: u64, output: *mut u8) -> bool {
    check_null!(input, output);
    let input_slice = unsafe { slice::from_raw_parts(input, len as usize) };
    let output_slice = unsafe { slice::from_raw_parts_mut(output, SHA3_512_OUT_SIZE) };

    let mut hasher = Sha3_512::new();
    hasher.update(input_slice);
    output_slice.copy_from_slice(&hasher.finalize());
    true
}

#[no_mangle]
pub extern "C" fn encrypt_aes_gcm_256(
    key: *const u8,
    iv: *const u8,
    aad: *const u8,
    aad_len: u64,
    data: *mut u8,
    data_len: u64,
    tag_out: *mut u8,
) -> bool {
    check_null!(key, iv, data, tag_out);

    if aad_len > 0 {
        check_null!(aad);
    }

    let key_slice = unsafe { slice::from_raw_parts(key, AES256_KEY_SIZE) };
    let iv_slice = unsafe { slice::from_raw_parts(iv, AES256_IV_SIZE) };
    let aad_slice = if aad.is_null() {
        &[]
    } else {
        unsafe { slice::from_raw_parts(aad, aad_len as usize) }
    };
    let data_slice = unsafe { slice::from_raw_parts_mut(data, data_len as usize) };

    let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(key_slice));
    let nonce = Nonce::from_slice(iv_slice);

    match cipher.encrypt_in_place_detached(nonce, aad_slice, data_slice) {
        Ok(tag) => {
            let out_tag = unsafe { slice::from_raw_parts_mut(tag_out, AES256_TAG_SIZE) };
            out_tag.copy_from_slice(&tag);
            true
        }
        Err(_) => false,
    }
}

#[no_mangle]
pub extern "C" fn decrypt_aes_gcm_256(
    key: *const u8,
    iv: *const u8,
    aad: *const u8,
    aad_len: u64,
    tag: *const u8,
    data: *mut u8,
    data_len: u64,
) -> bool {
    check_null!(key, iv, tag, data);

    if aad_len > 0 {
        check_null!(aad);
    }

    let key_slice = unsafe { slice::from_raw_parts(key, AES256_KEY_SIZE) };
    let iv_slice = unsafe { slice::from_raw_parts(iv, AES256_IV_SIZE) };
    let aad_slice = if aad.is_null() {
        &[]
    } else {
        unsafe { slice::from_raw_parts(aad, aad_len as usize) }
    };
    let tag_slice = unsafe { slice::from_raw_parts(tag, AES256_TAG_SIZE) };
    let data_slice = unsafe { slice::from_raw_parts_mut(data, data_len as usize) };

    let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(key_slice));
    let nonce = Nonce::from_slice(iv_slice);
    let gcm_tag = Tag::from_slice(tag_slice);

    cipher
        .decrypt_in_place_detached(nonce, aad_slice, data_slice, gcm_tag)
        .is_ok()
}

#[no_mangle]
pub extern "C" fn argon2id_hash(
    password: *const u8,
    pass_len: u64,
    salt: *const u8,
    salt_len: u64,
    output: *mut u8,
) -> bool {
    check_null!(password, salt, output);
    let pwd = unsafe { slice::from_raw_parts(password, pass_len as usize) };
    let slt = unsafe { slice::from_raw_parts(salt, salt_len as usize) };
    let out = unsafe { slice::from_raw_parts_mut(output, ARGON2_OUT_SIZE) };

    let argon2 = Argon2::default();
    argon2.hash_password_into(pwd, slt, out).is_ok()
}

#[no_mangle]
pub extern "C" fn secure_set_zero(ptr: *mut u8, len: u64) -> bool {
    if ptr.is_null() {
        return false;
    }
    let data = unsafe { slice::from_raw_parts_mut(ptr, len as usize) };
    data.zeroize();
    true
}

// --- Tests ---

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sha3_256_null_parameters() {
        let input = b"data";
        let mut output = [0u8; SHA3_256_OUT_SIZE];

        // input null
        assert!(!sha3_256_hash(
            std::ptr::null(),
            input.len() as u64,
            output.as_mut_ptr()
        ));

        // output null
        assert!(!sha3_256_hash(
            input.as_ptr(),
            input.len() as u64,
            std::ptr::null_mut()
        ));
    }

    #[test]
    fn test_sha3_512_null_parameters() {
        let input = b"data";
        let mut output = [0u8; SHA3_512_OUT_SIZE];

        // input null
        assert!(!sha3_512_hash(
            std::ptr::null(),
            input.len() as u64,
            output.as_mut_ptr()
        ));

        // output null
        assert!(!sha3_512_hash(
            input.as_ptr(),
            input.len() as u64,
            std::ptr::null_mut()
        ));
    }

    #[test]
    fn test_null_pointers() {
        // All should return false if they receive null pointers
        assert!(!sha3_256_hash(std::ptr::null(), 0, std::ptr::null_mut()));
        assert!(!argon2id_hash(
            std::ptr::null(),
            0,
            std::ptr::null(),
            0,
            std::ptr::null_mut()
        ));
        assert!(!secure_set_zero(std::ptr::null_mut(), 10));
    }

    #[test]
    fn test_sha3_512() {
        let input = b"rust";
        let mut output = [0u8; SHA3_512_OUT_SIZE];
        let res = sha3_512_hash(input.as_ptr(), input.len() as u64, output.as_mut_ptr());
        assert!(res);
        // Verify output is not all zeros
        assert!(output.iter().any(|&x| x != 0));
    }

    #[test]
    fn test_encrypt_null_parameters() {
        let key = [1u8; AES256_KEY_SIZE];
        let iv = [2u8; AES256_IV_SIZE];
        let aad = [3u8; 8];
        let mut data = *b"secret data";
        let mut tag = [0u8; AES256_TAG_SIZE];

        // key null
        assert!(!encrypt_aes_gcm_256(
            std::ptr::null(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            data.as_mut_ptr(),
            data.len() as u64,
            tag.as_mut_ptr()
        ));

        // iv null
        assert!(!encrypt_aes_gcm_256(
            key.as_ptr(),
            std::ptr::null(),
            std::ptr::null(),
            0,
            data.as_mut_ptr(),
            data.len() as u64,
            tag.as_mut_ptr()
        ));

        // data null
        assert!(!encrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            std::ptr::null_mut(),
            data.len() as u64,
            tag.as_mut_ptr()
        ));

        // tag_out null
        assert!(!encrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            data.as_mut_ptr(),
            data.len() as u64,
            std::ptr::null_mut()
        ));

        // aad_len > 0 pero aad null
        assert!(!encrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            aad.len() as u64,
            data.as_mut_ptr(),
            data.len() as u64,
            tag.as_mut_ptr()
        ));
    }

    #[test]
    fn test_decrypt_null_parameters() {
        let key = [1u8; AES256_KEY_SIZE];
        let iv = [2u8; AES256_IV_SIZE];
        let aad = [3u8; 8];
        let mut data = *b"secret data";
        let tag = [9u8; AES256_TAG_SIZE];

        // key null
        assert!(!decrypt_aes_gcm_256(
            std::ptr::null(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            tag.as_ptr(),
            data.as_mut_ptr(),
            data.len() as u64
        ));

        // iv null
        assert!(!decrypt_aes_gcm_256(
            key.as_ptr(),
            std::ptr::null(),
            std::ptr::null(),
            0,
            tag.as_ptr(),
            data.as_mut_ptr(),
            data.len() as u64
        ));

        // tag null
        assert!(!decrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            std::ptr::null(),
            data.as_mut_ptr(),
            data.len() as u64
        ));

        // data null
        assert!(!decrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            tag.as_ptr(),
            std::ptr::null_mut(),
            data.len() as u64
        ));

        // aad_len > 0 pero aad null
        assert!(!decrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            aad.len() as u64,
            tag.as_ptr(),
            data.as_mut_ptr(),
            data.len() as u64
        ));
    }

    #[test]
    fn test_aes_gcm_roundtrip() {
        let key = [1u8; 32];
        let iv = [2u8; 12];
        let mut data = *b"super secret message";
        let mut tag = [0u8; 16];

        // Encrypt
        let enc_res = encrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            data.as_mut_ptr(),
            data.len() as u64,
            tag.as_mut_ptr(),
        );
        assert!(enc_res);
        assert_ne!(&data, b"super secret message"); // Data must have changed

        // Decrypt
        let dec_res = decrypt_aes_gcm_256(
            key.as_ptr(),
            iv.as_ptr(),
            std::ptr::null(),
            0,
            tag.as_ptr(),
            data.as_mut_ptr(),
            data.len() as u64,
        );
        assert!(dec_res);
        assert_eq!(&data, b"super secret message"); // Data must be back to original
    }

    #[test]
    fn test_argon2id_null_parameters() {
        let pass = b"password";
        let salt = b"salt";
        let mut out = [0u8; ARGON2_OUT_SIZE];

        // password null
        assert!(!argon2id_hash(
            std::ptr::null(),
            pass.len() as u64,
            salt.as_ptr(),
            salt.len() as u64,
            out.as_mut_ptr()
        ));

        // salt null
        assert!(!argon2id_hash(
            pass.as_ptr(),
            pass.len() as u64,
            std::ptr::null(),
            salt.len() as u64,
            out.as_mut_ptr()
        ));

        // output null
        assert!(!argon2id_hash(
            pass.as_ptr(),
            pass.len() as u64,
            salt.as_ptr(),
            salt.len() as u64,
            std::ptr::null_mut()
        ));
    }

    #[test]
    fn test_argon2id() {
        let pass = b"password";
        let salt = b"somesalt";
        let mut out = [0u8; ARGON2_OUT_SIZE];
        assert!(argon2id_hash(
            pass.as_ptr(),
            pass.len() as u64,
            salt.as_ptr(),
            salt.len() as u64,
            out.as_mut_ptr()
        ));
    }

    #[test]
    fn test_secure_set_zero_null_parameter() {
        // ptr null
        assert!(!secure_set_zero(std::ptr::null_mut(), 16));
    }

    #[test]
    fn test_zeroize() {
        let mut sensitive_data = [5u8; 10];
        secure_set_zero(sensitive_data.as_mut_ptr(), sensitive_data.len() as u64);
        assert_eq!(sensitive_data, [0u8; 10]);
    }
}
