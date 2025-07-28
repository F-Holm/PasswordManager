add_library(gestor_contra STATIC
    gestor_contra.cc
)

target_include_directories(gestor_contra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gestor_contra PUBLIC
    utils
    datos
)