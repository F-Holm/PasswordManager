add_library(datos
    datos.h
    datos.cpp
)

target_include_directories(datos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})