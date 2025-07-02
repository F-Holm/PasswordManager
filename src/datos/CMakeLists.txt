add_library(datos
    datos.hh
    datos.cc
)

target_include_directories(datos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})