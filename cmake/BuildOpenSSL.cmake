FetchContent_Declare(
    openssl-cmake
    URL https://github.com/jimmy-park/openssl-cmake/archive/main.tar.gz
    CMAKE_CACHE_ARGS
        -OPENSSL_BUILD_TARGET:STRING=OFF
        -OPENSSL_TARGET_PLATFORM:STRING=ON
)
FetchContent_MakeAvailable(openssl-cmake)

target_link_libraries(openssl INTERFACE
    OpenSSL::SSL
    OpenSSL::Crypto
    OpenSSL::applink
)