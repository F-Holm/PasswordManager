include(FetchContent)

FetchContent_Declare(
    openssl-cmake
    URL https://github.com/jimmy-park/openssl-cmake/archive/main.tar.gz
)
FetchContent_MakeAvailable(openssl-cmake)

add_library(openssl_lib
    aes.cc
    sha.cc
)

target_include_directories(openssl_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(openssl PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    OpenSSL::applink
)