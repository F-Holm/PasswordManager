add_executable(datos_test
    datos_test.cc
)

target_link_libraries(datos_test PRIVATE
    cuenta    
    datos
    utils
)

AddTests(datos_test)

Format(datos_test .)
