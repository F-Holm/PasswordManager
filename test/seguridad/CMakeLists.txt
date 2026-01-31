add_executable(seguridad_test
    seguridad_test.cc
)

target_link_libraries(seguridad_test PRIVATE
    seguridad
)

AddTests(seguridad_test)

Format(seguridad_test .)
