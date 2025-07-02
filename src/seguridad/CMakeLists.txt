add_library(seguridad
    seguridad.hh
    aes.cc
    sha.cc
)

target_include_directories(seguridad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
