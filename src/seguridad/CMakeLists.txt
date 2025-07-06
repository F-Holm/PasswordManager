add_library(seguridad
    seguridad.cc
)

target_include_directories(seguridad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(seguridad PUBLIC
    openssl_
)