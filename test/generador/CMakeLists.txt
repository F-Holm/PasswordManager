add_executable(generador_test
    generador_test.cc
)

target_link_libraries(generador_test PRIVATE
    generador
)

AddTests(generador_test)

Format(generador_test .)