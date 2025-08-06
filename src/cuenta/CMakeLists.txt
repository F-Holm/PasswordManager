add_library(cuenta STATIC
    cuenta.cc
)

target_include_directories(cuenta PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(cuenta
    PUBLIC data_block
    PUBLIC db
    PRIVATE seguridad
)

Format(cuenta .)
AddCppCheck(cuenta)