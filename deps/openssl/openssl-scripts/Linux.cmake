function(build_openssl BUILD_DIR TYPE)
    set(OPENSSL_SRC_DIR "${CMAKE_SOURCE_DIR}/deps/openssl/openssl-src")
    set(OPENSSL_CONF_FLAGS "--prefix=${BUILD_DIR}" TYPE)

    file(MAKE_DIRECTORY "${BUILD_DIR}")

    message(STATUS "🔧 Configurando OpenSSL (${TYPE}) para ${OPENSSL_ARCH}")
    message(STATUS "🔧 Target: ${OPENSSL_TARGET}")
    message(STATUS "🔧 Instalando en: ${BUILD_DIR}")

    execute_process(
        COMMAND perl ./Configure ${OPENSSL_TARGET} ${OPENSSL_CONF_FLAGS}
        WORKING_DIRECTORY "${OPENSSL_SRC_DIR}"
        RESULT_VARIABLE configure_result
    )
    if(NOT configure_result EQUAL 0)
        message(FATAL_ERROR "Error al configurar OpenSSL")
    endif()

    execute_process(
        COMMAND make -j$(nproc)
        WORKING_DIRECTORY "${OPENSSL_SRC_DIR}"
        RESULT_VARIABLE make_result
    )
    if(NOT make_result EQUAL 0)
        message(FATAL_ERROR "Error al compilar OpenSSL")
    endif()

    execute_process(
        COMMAND make install_sw
        WORKING_DIRECTORY "${OPENSSL_SRC_DIR}"
        RESULT_VARIABLE install_result
    )
    if(NOT install_result EQUAL 0)
        message(FATAL_ERROR "Error al instalar OpenSSL")
    endif()

    message(STATUS "✅ OpenSSL (${TYPE}) compilado en ${BUILD_DIR}")
endfunction()

if (ARCH STREQUAL  "x64")
    set(OPENSSL_ARCH "linux-x86_64")
elseif(ARCH STREQUAL  "x86")
    set(OPENSSL_ARCH "linux-elf")
elseif(ARCH STREQUAL  "arm64")
    set(OPENSSL_ARCH "linux-aarch64")
endif()

set(OPENSSL_STATIC_DIR "${PROJECT_BINARY_DIR}/deps/openssl/openssl-static")
set(OPENSSL_SHARED_DIR "${PROJECT_BINARY_DIR}/deps/openssl/openssl-shared")

if (NOT (EXISTS "${OPENSSL_STATIC_DIR}" AND IS_DIRECTORY "${OPENSSL_STATIC_DIR}"))
    build_openssl(${OPENSSL_STATIC_DIR} "no-shared")
endif()
if (NOT (EXISTS "${OPENSSL_SHARED_DIR}" AND IS_DIRECTORY "${OPENSSL_SHARED_DIR}"))
    build_openssl(${OPENSSL_STATIC_DIR} "shared")
endif()
