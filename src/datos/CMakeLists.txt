add_library(datos
    datos.cc
)

target_include_directories(datos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(datos PUBLIC
    cuenta
    utils
)