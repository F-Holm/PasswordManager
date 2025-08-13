add_executable(GestorContra
    bootstrap.cc
)

target_link_libraries(GestorContra
    PRIVATE gestor_contra
)

Format(GestorContra .)
AddCppCheck(GestorContra)
AddClangTidy(GestorContra)