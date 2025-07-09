if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(openssl-scripts/Linux.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(openssl-scripts/Windows.cmake)
endif()

#add_library(openssl INTERFACE)

#target_link_libraries(openssl INTERFACE
#    OpenSSL::SSL
#    OpenSSL::Crypto
#)