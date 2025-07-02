include(FetchContent)

if(NOT DEFINED OPENSSL_VERSION)
    set(OPENSSL_VERSION openssl-3)
endif()

message(STATUS "Fetching OpenSSL version: ${OPENSSL_VERSION}")

# Download
FetchContent_Declare(
  openssl
  GIT_REPOSITORY https://github.com/openssl/openssl.git
  GIT_TAG        ${OPENSSL_VERSION}
)

# Opciones de build para OpenSSL
set(OPENSSL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPENSSL_BUILD_DOC     OFF CACHE BOOL "" FORCE)
set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)

# Build
message(STATUS "Building OpenSSL")
FetchContent_MakeAvailable(openssl)

# Exportar los targets
set(OPENSSL_TARGETS OpenSSL::SSL OpenSSL::Crypto PARENT_SCOPE)
