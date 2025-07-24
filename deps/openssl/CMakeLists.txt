include(ExternalProject)

# Config
set(OPENSSL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src")
set(OPENSSL_BUILD_DIR "${PROJECT_BINARY_DIR}/deps_/openssl")

set(OPENSSL_ENV_VARS "export CC=${CMAKE_C_COMPILER}")
set(OPENSSL_FLAGS_BASE "no-shared no-tests no-apps no-docs")
set(OPENSSL_CONFIGURAR "perl Configure ${OPENSSL_ARCH_TARGET} ${OPENSSL_FLAGS_BASE} --prefix=${OPENSSL_BUILD_DIR}")
set(OPENSSL_COMPILAR [[make -j$(nproc)]])
set(OPENSSL_INSTALAR "make install_sw")
set(OPENSSL_LIMPIAR "make clean")

execute_process(
  COMMAND "bash -c ${OPENSSL_ENV_VARS} && ${OPENSSL_CONFIGURAR} && ${OPENSSL_COMPILAR} && ${OPENSSL_INSTALAR} && ${OPENSSL_LIMPIAR}"
  WORKING_DIRECTORY ${OPENSSL_SOURCE_DIR}
  RESULT_VARIABLE OPENSSL_BUILD_RESULT
)

if(NOT OPENSSL_BUILD_RESULT EQUAL 0)
  message(FATAL_ERROR "OpenSSL build falló")
endif()
