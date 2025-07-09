set(OPENSSL_SCRIPTS ${CMAKE_SOURCE_DIR}/deps/openssl/openssl-scripts)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${OPENSSL_SCRIPTS}/Linux.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(${OPENSSL_SCRIPTS}/Windows.cmake)
endif()

#add_library(openssl INTERFACE)

#target_link_libraries(openssl INTERFACE
#    OpenSSL::SSL
#    OpenSSL::Crypto
#)