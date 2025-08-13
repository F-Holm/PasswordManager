add_library(seguridad STATIC
    seguridad.cc
)

target_include_directories(seguridad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(seguridad
    PRIVATE openssl_adapter
)

Format(seguridad .)
AddCppCheck(seguridad)
AddClangTidy(seguridad)