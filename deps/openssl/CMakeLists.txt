set(OPENSSL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src)
set(OPENSSL_BUILD_DIR ${CMAKE_BINARY_DIR}/_deps/openssl-build)
set(OPENSSL_STATIC_DIR "${PROJECT_BINARY_DIR}/deps/openssl-static")
set(OPENSSL_SHARED_DIR "${PROJECT_BINARY_DIR}/deps/openssl-shared")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND ARCH STREQUAL  "x64")
    set(OPENSSL_ARCH "linux-x86_64")
    set(STATIC_EXT "a")
    set(SHARED_EXT "so")
    set(OPENSSL_LIB_DIR "lib64")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND ARCH STREQUAL  "x86")
    set(OPENSSL_ARCH "linux-elf")
    set(STATIC_EXT "a")
    set(SHARED_EXT "so")
    set(OPENSSL_LIB_DIR "lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND ARCH STREQUAL  "arm64")
    set(OPENSSL_ARCH "linux-aarch64")
    set(STATIC_EXT "a")
    set(SHARED_EXT "so")
    set(OPENSSL_LIB_DIR "lib")
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
    make -j$(nproc) &&
    make install_sw &&
    make clean &&
    ${CMAKE_COMMAND} -E echo "Compilando OpenSSL shared" &&
    perl Configure ${OPENSSL_ARCH} shared --prefix=${OPENSSL_SHARED_DIR} &&
    make -j$(nproc) &&
    make install_sw &&
    make clean
  BUILD_ALWAYS 0 
)

add_library(openssl_static INTERFACE)
add_library(openssl_shared INTERFACE)

target_include_directories(openssl_static INTERFACE "${OPENSSL_STATIC_DIR}/include")
target_include_directories(openssl_shared INTERFACE "${OPENSSL_SHARED_DIR}/include")

target_link_libraries(openssl_static INTERFACE
  "${OPENSSL_STATIC_DIR}/${OPENSSL_LIB_DIR}/libssl.${STATIC_EXT}"
  "${OPENSSL_STATIC_DIR}/${OPENSSL_LIB_DIR}/libcrypto.${STATIC_EXT}"
)

target_link_libraries(openssl_shared INTERFACE
  "${OPENSSL_SHARED_DIR}/${OPENSSL_LIB_DIR}/libssl.${SHARED_EXT}"
  "${OPENSSL_SHARED_DIR}/${OPENSSL_LIB_DIR}/libcrypto.${SHARED_EXT}"
)

add_dependencies(openssl_static openssl)
add_dependencies(openssl_shared openssl)