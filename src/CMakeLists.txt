add_subdirectory(cuenta)
add_subdirectory(data_block)
add_subdirectory(datos)
add_subdirectory(db)
add_subdirectory(generador)
add_subdirectory(gestor_contra)
add_subdirectory(seguridad)

add_library(utils
    utils/set_rand.cc
)

target_include_directories(utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/utils")