include(GetOpenSSL)

add_library(openssl_
    aes.cc
    sha.cc
)

target_include_directories(openssl_ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(openssl_ PUBLIC OPENSSL_TARGETS)