add_executable(cuenta_test
    cuenta_test.cc
)

target_link_libraries(cuenta_test PRIVATE
    data_block
    cuenta
)

AddTests(cuenta_test)

Format(cuenta_test .)
