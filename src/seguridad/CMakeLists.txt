include(OpenSSL)

add_library(seguridad
    aes.cc
    sha.cc
)

target_include_directories(seguridad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(seguridad PUBLIC ${OPENSSL_TARGETS})