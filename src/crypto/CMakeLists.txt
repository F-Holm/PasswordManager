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

enable_rust_clippy(rust_crypto "${CMAKE_CURRENT_SOURCE_DIR}")
enable_rust_tests(rust_crypto "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(crypto STATIC
    src/crypto.cc
)

target_include_directories(crypto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(crypto PUBLIC rust_crypto)

ConfigModule(crypto)
