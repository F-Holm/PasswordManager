set(OPENSSL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src")
set(OPENSSL_BUILD_DIR "${PROJECT_BINARY_DIR}/deps_/openssl")

# Configurar comandos de compilación
set(OPENSSL_ENV_VARS "export CC=${CMAKE_C_COMPILER}")
set(OPENSSL_FLAGS_BASE "shared no-tests no-apps no-docs")
set(OPENSSL_CONFIGURAR "perl Configure ${OPENSSL_ARCH_TARGET} ${OPENSSL_FLAGS_BASE} --prefix=${OPENSSL_BUILD_DIR}")
set(OPENSSL_COMPILAR [[make -j$(nproc)]])
set(OPENSSL_INSTALAR "make install_sw")
set(OPENSSL_LIMPIAR "make clean")

# Compilación
message(Compilando OpenSSL...)
execute_process(
  COMMAND "bash -c ${OPENSSL_ENV_VARS} && ${OPENSSL_CONFIGURAR} && ${OPENSSL_COMPILAR} && ${OPENSSL_INSTALAR} && ${OPENSSL_LIMPIAR}"
  WORKING_DIRECTORY ${OPENSSL_SOURCE_DIR}
  RESULT_VARIABLE OPENSSL_BUILD_RESULT
)

if(NOT OPENSSL_BUILD_RESULT EQUAL 0)
  message(FATAL_ERROR "OpenSSL build falló")
endif()

# Configurar ruta librerías
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(STATIC_EXT "a")
    set(SHARED_EXT "dll")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(STATIC_EXT "a")
    set(SHARED_EXT "so")
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(OPENSSL_LIB_DIR "${OPENSSL_BUILD_DIR}/lib64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
    set(OPENSSL_LIB_DIR "${OPENSSL_BUILD_DIR}/lib")
endif()

# Crear librerías importadas estáticas y compartidas
add_library(openssl_static INTERFACE)
add_library(openssl_shared INTERFACE)

target_include_directories(openssl_static INTERFACE "${OPENSSL_STATIC_DIR}/include")
target_include_directories(openssl_shared INTERFACE "${OPENSSL_SHARED_DIR}/include")

target_link_libraries(openssl_static INTERFACE
  "${OPENSSL_LIB_DIR}/libssl.${STATIC_EXT}"
  "${OPENSSL_LIB_DIR}/libcrypto.${STATIC_EXT}"
)

target_link_libraries(openssl_shared INTERFACE
  "${OPENSSL_LIB_DIR}/libssl.${SHARED_EXT}"
  "${OPENSSL_LIB_DIR}/libcrypto.${SHARED_EXT}"
)