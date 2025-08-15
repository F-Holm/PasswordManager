find_package(OpenSSL REQUIRED)

add_library(openssl_adapter SHARED
    aes.cc
    sha.cc
)

target_include_directories(openssl_adapter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

LinkOpenSSL(openssl_adapter)

InstrumentForCoverage(openssl_adapter)
CleanCoverage(openssl_adapter)
Format(openssl_adapter .)
AddCppCheck(openssl_adapter)
AddClangTidy(openssl_adapter)