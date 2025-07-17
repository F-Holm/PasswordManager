add_library(datos STATIC
    datos.cc
)

target_include_directories(datos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(datos PRIVATE
    cuenta
)