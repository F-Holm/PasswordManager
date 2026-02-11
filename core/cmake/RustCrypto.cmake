if (WIN32)
    set(CRYPTO_LIB_NAME "librust_crypto.lib")
else()
    set(CRYPTO_LIB_NAME "librust_crypto.a")
endif()
set(CRYPTO_HEADER_NAME "rust_crypto.h")

set(CRYPTO_SRC_DIR "${CMAKE_SOURCE_DIR}/../crypto")
set(CRYPTO_BINARY_DIR_ "${CRYPTO_SRC_DIR}/target/release")

set(CRYPTO_BINARY_PATH "${CRYPTO_BINARY_DIR_}/${CRYPTO_LIB_NAME}")
set(CRYPTO_HEADER_PATH "${CRYPTO_BINARY_DIR_}/${CRYPTO_HEADER_NAME}")

add_custom_command(
    OUTPUT "${CRYPTO_BINARY_PATH}"
    COMMAND cargo build --release
    WORKING_DIRECTORY "${CRYPTO_SRC_DIR}"
    COMMENT "Compiling Rust Crypto module..."
    VERBATIM
)

add_custom_target(crypto_rust_build ALL DEPENDS "${CRYPTO_BINARY_PATH}")

add_library(rust_crypto STATIC IMPORTED)
set_target_properties(rust_crypto PROPERTIES
    IMPORTED_LOCATION "${CRYPTO_BINARY_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CRYPTO_BINARY_DIR_}>"
)

add_dependencies(rust_crypto crypto_rust_build)
