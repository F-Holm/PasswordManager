add_executable(gestor_contra_test
    gestor_contra_test.cc
)

target_link_libraries(gestor_contra_test PRIVATE gestor_contra)

AddTests(gestor_contra_test)