add_library(types STATIC)

add_library(types STATIC
    cuenta_arch.cc
    cuenta.cc
    ind_cuenta.cc
)

target_include_directories(types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

Link_TLS(types)

InstrumentForCoverage(types)
CleanCoverage(types)
Format(types .)
AddCppCheck(types)
AddClangTidy(types)
