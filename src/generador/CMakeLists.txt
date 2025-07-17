add_library(generador STATIC
    generador.cc
)

target_include_directories(generador PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})