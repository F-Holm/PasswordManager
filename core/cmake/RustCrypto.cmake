include(FetchContent)

FetchContent_Declare(
  corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.6.1
)

FetchContent_MakeAvailable(corrosion)

corrosion_import_crate(
    MANIFEST_PATH ${CMAKE_SOURCE_DIR}/../crypto/Cargo.toml
)

if (WIN32)
    target_link_libraries(rust_crypto INTERFACE bcrypt)
endif()

target_include_directories(rust_crypto
    INTERFACE
        ${CMAKE_SOURCE_DIR}/../crypto/include
)
