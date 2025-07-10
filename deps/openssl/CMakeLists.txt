set(OPENSSL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src)
set(OPENSSL_BUILD_DIR ${CMAKE_BINARY_DIR}/_deps/openssl-build)
set(OPENSSL_INSTALL_DIR ${CMAKE_BINARY_DIR}/_install/openssl)
set(OPENSSL_STATIC_DIR "${PROJECT_BINARY_DIR}/deps/openssl-static")
set(OPENSSL_SHARED_DIR "${PROJECT_BINARY_DIR}/deps/openssl-shared")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND ARCH STREQUAL  "x64")
    set(OPENSSL_ARCH "linux-x86_64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND ARCH STREQUAL  "x86")
    set(OPENSSL_ARCH "linux-elf")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND ARCH STREQUAL  "arm64")
    set(OPENSSL_ARCH "linux-aarch64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND ARCH STREQUAL  "x64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND ARCH STREQUAL  "x86")
else()
    message(FATAL_ERROR "❌ Arquitectura no soportada: ${ARCH}")
endif()

ExternalProject_Add(openssl
  SOURCE_DIR "${OPENSSL_SOURCE_DIR}"
  BINARY_DIR "${OPENSSL_BUILD_DIR}"
  BUILD_COMMAND 
    ${CMAKE_COMMAND} -E echo "Compilando OpenSSL static" &&
    perl Configure ${OPENSSL_ARCH} no-shared --prefix=${OPENSSL_STATIC_DIR} &&
    make -j$(nproc)
    make install_sw &&
    make clean &&
    ${CMAKE_COMMAND} -E echo "Compilando OpenSSL shared" &&
    perl Configure ${OPENSSL_ARCH} shared --prefix=${OPENSSL_SHARED_DIR} &&
    make -j$(nproc)
    make install_sw &&
    make clean
  BUILD_ALWAYS 0 
)

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