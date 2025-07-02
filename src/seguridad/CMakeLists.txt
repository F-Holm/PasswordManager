add_library(seguridad
    seguridad.h
    aes.cpp
    sha.cpp
)

target_include_directories(seguridad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
