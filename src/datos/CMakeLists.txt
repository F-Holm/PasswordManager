add_library(datos STATIC
    datos.cc
)

target_include_directories(datos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(datos
    PRIVATE types
    PUBLIC cuenta
)

InstrumentForCoverage(datos)
CleanCoverage(datos)
Format(datos .)
AddCppCheck(datos)
AddClangTidy(datos)