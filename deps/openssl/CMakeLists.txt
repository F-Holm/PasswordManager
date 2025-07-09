if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(openssl/Linux.cmake)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(openssl/Windows.cmake)
endif()

add_library(openssl INTERFACE)

target_link_libraries(openssl INTERFACE
    OpenSSL::SSL
    OpenSSL::Crypto
)