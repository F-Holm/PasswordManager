add_library(generador STATIC
    generador.cc
)

target_include_directories(generador PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

Format(generador .)
AddCppCheck(generador)
AddClangTidy(generador)