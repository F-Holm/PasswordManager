include(ExternalProject)

# Config
set(OPENSSL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src")
set(OPENSSL_STATIC_DIR "${PROJECT_BINARY_DIR}/deps_/openssl-static")
set(OPENSSL_SHARED_DIR "${PROJECT_BINARY_DIR}/deps_/openssl-shared")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(OPENSSL_ARCH "mingw64")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    set(OPENSSL_ARCH "mingw")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(OPENSSL_ARCH "mingw64-aarch64")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
    set(OPENSSL_ARCH "mingw-armv7")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(OPENSSL_ARCH "linux-x86_64")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
    set(OPENSSL_ARCH "linux-elf")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(OPENSSL_ARCH "linux-aarch64")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
    set(OPENSSL_ARCH "linux-armv4")

else()
    message(FATAL_ERROR "❌ Arquitectura no soportada: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(STATIC_EXT "a")
    set(SHARED_EXT "dll")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(STATIC_EXT "a")
    set(SHARED_EXT "so")
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(OPENSSL_LIB_DIR "lib64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
    set(OPENSSL_LIB_DIR "lib")
endif()

# perl Configure LIST | grep linux
# perl Configure LIST | grep mingw

# export CC="clang"
# unset CC

# perl Configure linux-x86_64 no-shared --prefix=/home/zlatan/openssl/openssl-linux-x64-static && grep ^CC= Makefile && make -j$(nproc) && make install_sw && make clean
# perl Configure linux-x86 shared --cross-compile-prefix=i686-linux-gnu- --prefix=/home/zlatan/openssl/openssl-linux-x86-shared && grep ^CC= Makefile && make -j$(nproc) && make install_sw && make clean

# ./Configure linux-x86_64 no-tests no-apps no-docs --prefix=/ruta/instalacion

# Build
set(OPENSSL_BUILD_COMMAND
    "${CMAKE_COMMAND} -E echo "Compilando OpenSSL static" &&
    perl Configure ${OPENSSL_ARCH} no-shared --prefix=${OPENSSL_STATIC_DIR} &&
    make -j &&
    make install_sw &&
    make clean &&
    ${CMAKE_COMMAND} -E echo "Compilando OpenSSL shared" &&
    perl Configure ${OPENSSL_ARCH} shared --prefix=${OPENSSL_SHARED_DIR} &&
    make -j &&
    make install_sw &&
    make clean"
)

ExternalProject_Add(openssl
  SOURCE_DIR "${OPENSSL_SOURCE_DIR}"
  BUILD_COMMAND "${OPENSSL_BUILD_COMMAND}"
  BUILD_ALWAYS 0 
)

# Link
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