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

set(RUST_NEW_DIR "${CMAKE_BINARY_DIR}/rust") 
set(CRYPTO_NEW_DIR "${RUST_NEW_DIR}/rust_crypto")
set(CRYPTO_BINARY_NEW_PATH "${CRYPTO_NEW_DIR}/${CRYPTO_LIB_NAME}")
set(CRYPTO_HEADER_NEW_PATH "${CRYPTO_NEW_DIR}/${CRYPTO_HEADER_NAME}")

file(MAKE_DIRECTORY ${CRYPTO_NEW_DIR})

add_custom_command(
    OUTPUT ${CRYPTO_BINARY_NEW_PATH}
    COMMAND cargo build --release
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CRYPTO_NEW_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CRYPTO_BINARY_PATH}"
            "${CRYPTO_BINARY_NEW_PATH}"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CRYPTO_HEADER_PATH}"
            "${CRYPTO_HEADER_NEW_PATH}"
    WORKING_DIRECTORY "${CRYPTO_SRC_DIR}"
    COMMENT "Building Rust crypto library"
)

add_custom_target(rust_crypto_build ALL
    DEPENDS ${CRYPTO_BINARY_NEW_PATH}
)

add_library(rust_crypto STATIC IMPORTED)
add_dependencies(rust_crypto rust_crypto_build)

set_target_properties(rust_crypto PROPERTIES
    IMPORTED_LOCATION "${CRYPTO_BINARY_NEW_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${RUST_NEW_DIR}"
)
