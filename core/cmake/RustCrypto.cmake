if (WIN32) {
    set(CRYPTO_LIB_NAME "libmy_crypto.lib")
} else {
    set(CRYPTO_LIB_NAME "libmy_crypto.a")
}

set(CRYPTO_SRC_DIR "${CMAKE_SOURCE_DIR}/../crypto")
set(CRYPTO_BINARY_PATH "${CRYPTO_SRC_DIR}/target/release/${CRYPTO_LIB_NAME}")

add_custom_command(
    OUTPUT "${CRYPTO_BINARY_PATH}"
    COMMAND cargo build --release
    WORKING_DIRECTORY "${CRYPTO_SRC_DIR}"
    COMMENT "Compiling Rust Crypto module ..."
    VERBATIM
)

add_custom_target(crypto_rust_build ALL DEPENDS "${CRYPTO_BINARY_PATH}")

add_library(rust_crypto_lib STATIC IMPORTED)
set_target_properties(rust_crypto_lib PROPERTIES
    IMPORTED_LOCATION "${CRYPTO_BINARY_PATH}"
)

add_dependencies(rust_crypto_lib crypto_rust_build)
