set(OPENSSL_REPO_DIR "${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src/")
set(OPENSSL_SOURCE_DIR "${PROJECT_BINARY_DIR}/_deps/openssl-src/")
set(OPENSSL_BUILD_DIR "${PROJECT_BINARY_DIR}/_deps/openssl-build")
set(OPENSSL_ARCHIVO_BUILD_EXITOSA "${OPENSSL_BUILD_DIR}/.build-exitosa")

verify_submodule_cloned("OpenSSL" "${OPENSSL_REPO_DIR}" "README.md")

# Configurar comandos de compilación
set(OPENSSL_ENV_VARS "export CC=${CMAKE_C_COMPILER}")
set(OPENSSL_FLAGS_BASE "shared no-tests no-apps no-docs")
set(OPENSSL_CONFIGURAR "perl Configure ${OPENSSL_ARCH_TARGET} ${OPENSSL_FLAGS_BASE} --prefix=${OPENSSL_BUILD_DIR}")
set(OPENSSL_COMPILAR [[make -j$(nproc)]])
set(OPENSSL_INSTALAR "make install_sw")
set(OPENSSL_LIMPIAR "make clean")

if(IS_DIRECTORY "${OPENSSL_BUILD_DIR}" AND EXISTS "${OPENSSL_ARCHIVO_BUILD_EXITOSA}")
    message(STATUS "OpenSSL ya está compilado")
else()
    if(IS_DIRECTORY "${OPENSSL_BUILD_DIR}")
        file(REMOVE_RECURSE "${OPENSSL_BUILD_DIR}")
    endif()
    if(IS_DIRECTORY "${OPENSSL_SOURCE_DIR}")
        file(REMOVE_RECURSE "${OPENSSL_SOURCE_DIR}")
    endif()
    # Compilación
    message(STATUS "Copiando OpenSSL...")
    file(MAKE_DIRECTORY "${OPENSSL_SOURCE_DIR}")
    file(COPY "${OPENSSL_REPO_DIR}/" DESTINATION "${OPENSSL_SOURCE_DIR}")
    message(STATUS "Compilando OpenSSL...")
    execute_process(
        COMMAND bash -c "${OPENSSL_ENV_VARS} && ${OPENSSL_CONFIGURAR} && ${OPENSSL_COMPILAR} && ${OPENSSL_INSTALAR} && ${OPENSSL_LIMPIAR}"
        WORKING_DIRECTORY ${OPENSSL_SOURCE_DIR}
        RESULT_VARIABLE OPENSSL_BUILD_RESULT
        OUTPUT_VARIABLE OPENSSL_BUILD_OUTPUT
        ERROR_VARIABLE OPENSSL_BUILD_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE 
        ERROR_STRIP_TRAILING_WHITESPACE 
    )

    if(NOT OPENSSL_BUILD_RESULT EQUAL 0)
        message("OpenSSL repo dir: ${OPENSSL_REPO_DIR}")
        message("OpenSSL source dir: ${OPENSSL_SOURCE_DIR}")
        message("OpenSSL build dir: ${OPENSSL_BUILD_DIR}")
        message("OpenSSL build output: ${OPENSSL_BUILD_OUTPUT}")
        message("OpenSSL build result: ${OPENSSL_BUILD_RESULT}")
        message("OpenSSL build error: ${OPENSSL_BUILD_ERROR}")
        message(FATAL_ERROR "OpenSSL build falló")
    endif()
endif()

file(WRITE "${OPENSSL_ARCHIVO_BUILD_EXITOSA}")

# Configurar ruta librerías
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(STATIC_EXT "a")
    set(SHARED_EXT "dll")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(STATIC_EXT "a")
    set(SHARED_EXT "so")
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(OPENSSL_LIB_DIR "${OPENSSL_BUILD_DIR}/lib64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(OPENSSL_LIB_DIR "${OPENSSL_BUILD_DIR}/lib")
endif()

# Crear librerías importadas estáticas y compartidas
add_library(openssl_static INTERFACE)
add_library(openssl_shared INTERFACE)

target_include_directories(openssl_static INTERFACE "${OPENSSL_BUILD_DIR}/include")
target_include_directories(openssl_shared INTERFACE "${OPENSSL_BUILD_DIR}/include")

target_link_libraries(openssl_static INTERFACE
    "${OPENSSL_LIB_DIR}/libssl.${STATIC_EXT}"
    "${OPENSSL_LIB_DIR}/libcrypto.${STATIC_EXT}"
)

target_link_libraries(openssl_shared INTERFACE
    "${OPENSSL_LIB_DIR}/libssl.${SHARED_EXT}"
    "${OPENSSL_LIB_DIR}/libcrypto.${SHARED_EXT}"
)