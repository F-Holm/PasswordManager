add_library(openssl_adapter STATIC
    aes.cc
    sha.cc
)

target_include_directories(openssl_adapter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(openssl_adapter
    PRIVATE openssl_static
)