add_library(generador STATIC
    generador.cc
)

target_include_directories(generador PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

InstrumentForCoverage(generador)
CleanCoverage(generador)
Format(generador .)
AddCppCheck(generador)
AddClangTidy(generador)
