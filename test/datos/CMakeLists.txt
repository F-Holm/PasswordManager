add_executable(datos_test
    datos_test.cc
)

target_link_libraries(datos_test PRIVATE
    datos
    cuenta
)

AddTests(datos_test)