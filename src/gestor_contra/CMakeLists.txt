add_executable(GestorContra gestor_contra.cc)

target_link_libraries(GestorContra PRIVATE
    utils
    datos
)