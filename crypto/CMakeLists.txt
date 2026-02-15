corrosion_import_crate(
    MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml
)

if (WIN32)
    target_link_libraries(rust_crypto INTERFACE bcrypt)
endif()

target_include_directories(rust_crypto
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(
    NAME rust_crypto_test
    COMMAND cargo test --manifest-path "${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml"
)

enable_rust_fmt(rust_crypto "${CMAKE_CURRENT_SOURCE_DIR}")
enable_rust_clippy(rust_crypto "${CMAKE_CURRENT_SOURCE_DIR}")
