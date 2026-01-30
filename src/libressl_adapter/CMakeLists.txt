add_library(libressl_adapter STATIC
    aes.cc
    sha.cc
)

target_include_directories(libressl_adapter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

Link_TLS(libressl_adapter)

InstrumentForCoverage(libressl_adapter)
CleanCoverage(libressl_adapter)
Format(libressl_adapter .)
AddCppCheck(libressl_adapter)
AddClangTidy(libressl_adapter)