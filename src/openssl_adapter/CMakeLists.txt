add_library(openssl_adapter
    aes.cc
    sha.cc
)

target_include_directories(openssl_adapter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})