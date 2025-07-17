add_library(seguridad STATIC
    seguridad.cc
)

target_include_directories(seguridad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(seguridad PRIVATE
    openssl_adapter
)