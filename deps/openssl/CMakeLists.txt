set(OPENSSL_SCRIPTS ${CMAKE_SOURCE_DIR}/deps/openssl/openssl-scripts)
set(OPENSSL_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/openssl-src)
set(OPENSSL_BUILD_DIR ${CMAKE_BINARY_DIR}/_deps/openssl-build)
set(OPENSSL_INSTALL_DIR ${CMAKE_BINARY_DIR}/_install/openssl)


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${OPENSSL_SCRIPTS}/Linux.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(${OPENSSL_SCRIPTS}/Windows.cmake)
endif()

ExternalProject_Add(openssl
  GIT_REPOSITORY ...
  CONFIGURE_COMMAND ""  # Nada, lo hacés vos
  BUILD_COMMAND 
    ${CMAKE_COMMAND} -E echo "Compilando static" &&
    perl Configure linux-x86_64 no-shared --prefix=... &&
    make install_sw &&
    make clean &&
    ${CMAKE_COMMAND} -E echo "Compilando shared" &&
    perl Configure linux-x86_64 shared --prefix=... &&
    make install_sw
  ...
)

ExternalProject_Add(openssl_static
  PREFIX openssl_static
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG OpenSSL_1_1_1w  # o la versión que quieras
  SOURCE_DIR ${OPENSSL_SOURCE_DIR}
  CONFIGURE_COMMAND perl Configure linux-x86_64 no-shared --prefix=${OPENSSL_INSTALL_DIR}/static
  BUILD_COMMAND make -j
  INSTALL_COMMAND make install_sw
  BUILD_IN_SOURCE 1
)

# SHARED
ExternalProject_Add(openssl_shared
  PREFIX openssl_shared
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG OpenSSL_1_1_1w
  SOURCE_DIR ${OPENSSL_SOURCE_DIR}_shared
  CONFIGURE_COMMAND perl Configure linux-x86_64 shared --prefix=${OPENSSL_INSTALL_DIR}/shared
  BUILD_COMMAND make -j
  INSTALL_COMMAND make install_sw
  BUILD_IN_SOURCE 1
)


#add_library(openssl INTERFACE)

#target_link_libraries(openssl INTERFACE
#    OpenSSL::SSL
#    OpenSSL::Crypto
#)