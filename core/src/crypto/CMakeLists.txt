add_library(crypto STATIC
    aes_gcm_256.cc
    secure_clear.cc
    sha_256.cc
)

target_include_directories(crypto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

Link_TLS(crypto)

InstrumentForCoverage(crypto)
CleanCoverage(crypto)
Format(crypto .)
AddCppCheck(crypto)
AddClangTidy(crypto)
