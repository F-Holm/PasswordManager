add_library(cuenta
    cuenta.cc
)

target_include_directories(cuenta PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(cuenta PUBLIC
    db
    data_block
    seguridad
)