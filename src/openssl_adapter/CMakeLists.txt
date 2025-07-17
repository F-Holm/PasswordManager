add_library(openssl_adapter STATIC
    aes.cc
    sha.cc
)

target_include_directories(openssl_adapter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})