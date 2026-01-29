include(FetchContent)

FetchContent_Declare(
    libressl
    URL https://github.com/libressl/portable/releases/download/v4.2.1/libressl-4.2.1.tar.gz
)

# Opciones razonables
set(LIBRESSL_APPS OFF CACHE BOOL "" FORCE)
set(LIBRESSL_TESTS OFF CACHE BOOL "" FORCE)
set(LIBRESSL_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libressl)

# ------------------------------------------------------------
# Función de linkeo (lo mínimo necesario)
# ------------------------------------------------------------
function(Link_TLS target)
    target_link_libraries(${target}
        PRIVATE
            LibreSSL::Crypto
            LibreSSL::SSL
            LibreSSL::TLS
    )
endfunction()

