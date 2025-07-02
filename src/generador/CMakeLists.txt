add_library(generador
    generador.hh
    generador.cc
)

target_include_directories(generador PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})