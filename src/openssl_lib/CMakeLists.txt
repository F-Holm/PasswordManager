add_library(openssl_lib
    aes.cc
    sha.cc
)

target_include_directories(openssl_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#target_link_libraries(openssl PUBLIC
#)