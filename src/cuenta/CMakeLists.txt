add_library(cuenta STATIC
    cuenta.cc
)

target_include_directories(cuenta PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(cuenta PRIVATE
    db
    data_block
    seguridad
)