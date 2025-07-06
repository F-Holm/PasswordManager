include(FetchContent)

if(NOT DEFINED OPENSSL_VERSION)
    set(OPENSSL_VERSION openssl-3.5.1)
endif()

message(STATUS "Fetching OpenSSL version: ${OPENSSL_VERSION}")

# Download
FetchContent_Declare(
  OpenSSL
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG        ${OPENSSL_VERSION}
  GIT_SHALLOW    TRUE
)

# Opciones de build para OpenSSL
set(OPENSSL_BUILD_TESTS       OFF CACHE BOOL "" FORCE)
set(OPENSSL_BUILD_DOC         OFF CACHE BOOL "" FORCE)
set(OPENSSL_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(OPENSSL_NO_APPS           ON  CACHE BOOL "" FORCE)

# Build
message(STATUS "Building OpenSSL")
FetchContent_MakeAvailable(OpenSSL)

if (TARGET OpenSSL::SSL AND TARGET OpenSSL::Crypto)
    message(STATUS "OpenSSL ha sido compilado y los targets OpenSSL::SSL y OpenSSL::Crypto están disponibles.")
    message(STATUS "    - Biblioteca SSL disponible.")
    message(STATUS "    - Biblioteca Crypto disponible.")
else()
    message(WARNING "OpenSSL no se pudo compilar o los targets OpenSSL::SSL y/o OpenSSL::Crypto no se encontraron.")
    message(WARNING "Verifica la versión de GIT_TAG y los logs de compilación de OpenSSL para errores.")
endif()

# Exportar los targets
set(OPENSSL_TARGETS OpenSSL::SSL OpenSSL::Crypto PARENT_SCOPE)
